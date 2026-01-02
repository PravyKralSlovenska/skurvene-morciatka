#include "engine/renderer/world_renderer.hpp"

#include "engine/renderer/buffers/vertex_buffer_object.hpp"
#include "engine/renderer/buffers/vertex_array_object.hpp"
#include "engine/renderer/buffers/element_array_object.hpp"
#include "engine/renderer/buffers/shader_storage_buffer_object.hpp"
#include "engine/particle/particle.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/renderer/compute_shader.hpp"
#include "others/GLOBALS.hpp"
#include "others/utils.hpp"

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace
{
    bool log_gl_error(const char *label)
    {
        bool had_error = false;
        GLenum error = GL_NO_ERROR;
        while ((error = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "[WorldRenderer] GL error after " << label << ": 0x"
                      << std::hex << error << std::dec << '\n';
            had_error = true;
        }
        return had_error;
    }

    void dump_vertex_preview(GLuint buffer_id, std::size_t vertex_count)
    {
        if (buffer_id == 0 || vertex_count == 0)
        {
            return;
        }

        const std::size_t max_preview = std::min<std::size_t>(vertex_count, 6);

        struct DebugVertex
        {
            glm::vec2 position;
            glm::vec2 _padding;
            glm::vec4 color;
        };

        std::vector<DebugVertex> preview(max_preview);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, preview.size() * sizeof(DebugVertex), preview.data());
        log_gl_error("glGetBufferSubData vertex preview");

        std::size_t first_colored = preview.size();
        for (std::size_t i = 0; i < preview.size(); ++i)
        {
            if (preview[i].color.a > 0.0f)
            {
                first_colored = i;
                break;
            }
        }

        std::cerr << "[WorldRenderer] Vertex preview (" << max_preview << ")\n";
        for (std::size_t i = 0; i < preview.size(); ++i)
        {
            const auto &v = preview[i];
            std::cerr << "  [" << i << "] pos(" << v.position.x << ", " << v.position.y
                      << ") color(" << v.color.r << ", " << v.color.g << ", "
                      << v.color.b << ", " << v.color.a << ")";
            if (i == first_colored)
            {
                std::cerr << " <-- first non-empty";
            }
            std::cerr << '\n';
        }

        if (first_colored < preview.size())
        {
            const auto &v = preview[first_colored];
            std::cerr << "[WorldRenderer] Sample non-empty vertex: pos(" << v.position.x << ", " << v.position.y
                      << ") alpha=" << v.color.a << '\n';
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

World_Renderer::World_Renderer(World *world)
    : world(world) {}

World_Renderer::~World_Renderer()
{
}

void World_Renderer::init()
{
    dummy_VAO = std::make_unique<VERTEX_ARRAY_OBJECT>();

    render_shader = std::make_unique<Shader>("../shaders/compute/compute_vertex.glsl", "../shaders/compute/compute_fragment.glsl");

    compute_shader = std::make_unique<Compute_Shader>("../shaders/compute/compute_calculate_vertices.glsl");

    particle_ssbo = std::make_unique<Shader_Storage_Buffer_Object>();
    vertex_ssbo = std::make_unique<Shader_Storage_Buffer_Object>();
    chunk_ssbo = std::make_unique<Shader_Storage_Buffer_Object>();
    vertex_counter_ssbo = std::make_unique<Shader_Storage_Buffer_Object>();
}

void World_Renderer::set_world(World *world)
{
    this->world = world;
}

void World_Renderer::set_projection(glm::mat4 projection)
{
    this->projection = projection;
}

void World_Renderer::render_world_compute()
{
    static int frame_count = 0;
    static auto last_log_time = std::chrono::high_resolution_clock::now();
    ++frame_count;

    auto frame_start = std::chrono::high_resolution_clock::now();

    if (!world)
    {
        return;
    }

    const auto *active_chunks = world->get_active_chunks();
    if (!active_chunks || active_chunks->empty())
    {
        return;
    }

    const glm::ivec2 chunk_dimensions = world->get_chunk_dimensions();
    if (chunk_dimensions.x <= 0 || chunk_dimensions.y <= 0)
    {
        return;
    }

    // GPU-side structures matching the shader
    struct alignas(16) GPUWorldCell
    {
        glm::vec4 base_color;
        glm::vec4 color;
        glm::vec2 world_coords; // World position in pixels
        glm::uvec2 meta;        // [particle_type, other data]
    };

    struct GPUChunkInfo
    {
        glm::ivec2 coords;
        glm::ivec2 padding;
    };

    struct GPUVertex
    {
        glm::vec2 position; // 8 bytes
        glm::vec2 _padding; // 8 bytes padding for std430 alignment
        glm::vec4 color;    // 16 bytes, must be 16-byte aligned
    };

    constexpr int vertices_per_cell = 6;
    const int cells_per_chunk = chunk_dimensions.x * chunk_dimensions.y;

    // Upload ALL cells (including empty) - let GPU filter them
    std::vector<GPUWorldCell> gpu_cells;
    std::vector<GPUChunkInfo> gpu_chunks;

    // Extract visible bounds from view-projection matrix
    // The projection matrix transforms world coords to NDC [-1, 1]
    // We need to find world coordinates that map to screen edges
    glm::mat4 inv_projection = glm::inverse(projection);

    // Transform NDC corners to world space
    glm::vec4 ndc_corners[4] = {
        glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f), // bottom-left
        glm::vec4(1.0f, -1.0f, 0.0f, 1.0f),  // bottom-right
        glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),  // top-left
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)    // top-right
    };

    float min_visible_x = FLT_MAX;
    float max_visible_x = -FLT_MAX;
    float min_visible_y = FLT_MAX;
    float max_visible_y = -FLT_MAX;

    for (int i = 0; i < 4; ++i)
    {
        glm::vec4 world_pos = inv_projection * ndc_corners[i];
        world_pos /= world_pos.w; // perspective divide

        min_visible_x = std::min(min_visible_x, world_pos.x);
        max_visible_x = std::max(max_visible_x, world_pos.x);
        min_visible_y = std::min(min_visible_y, world_pos.y);
        max_visible_y = std::max(max_visible_y, world_pos.y);
    }

    // Add small margin to avoid pop-in (just a few particles worth)
    const float chunk_pixel_width = chunk_dimensions.x * Globals::PARTICLE_SIZE;
    const float chunk_pixel_height = chunk_dimensions.y * Globals::PARTICLE_SIZE;
    const float margin = Globals::PARTICLE_SIZE * 10.0f; // 10 particles margin instead of full chunk
    min_visible_x -= margin;
    max_visible_x += margin;
    min_visible_y -= margin;
    max_visible_y += margin;

    gpu_chunks.reserve(active_chunks->size());

    int culled_chunks = 0;
    int total_visible_cells = 0;

    // Build list of visible chunks and upload their raw data directly
    for (const auto &coords : *active_chunks)
    {
        // Frustum culling: check if chunk is visible
        float chunk_min_x = coords.x * chunk_pixel_width;
        float chunk_max_x = chunk_min_x + chunk_pixel_width;
        float chunk_min_y = coords.y * chunk_pixel_height;
        float chunk_max_y = chunk_min_y + chunk_pixel_height;

        // Skip chunk if completely outside visible area
        if (chunk_max_x < min_visible_x || chunk_min_x > max_visible_x ||
            chunk_max_y < min_visible_y || chunk_min_y > max_visible_y)
        {
            ++culled_chunks;
            continue;
        }

        Chunk *chunk = world->get_chunk(coords);
        if (!chunk)
        {
            continue;
        }

        const auto *chunk_data = chunk->get_chunk_data();
        if (!chunk_data || chunk_data->empty())
        {
            continue;
        }

        // Store chunk coords - shader will calculate world positions from chunk coords + cell index
        gpu_chunks.push_back({coords, glm::ivec2(0, 0)});

        // Batch-resize and fill - avoids repeated push_back allocations
        const size_t chunk_start_idx = gpu_cells.size();
        gpu_cells.resize(chunk_start_idx + cells_per_chunk);

        for (int i = 0; i < cells_per_chunk; ++i)
        {
            const Particle &p = (*chunk_data)[i].particle;
            const int cell_x = i % chunk_dimensions.x;
            const int cell_y = i / chunk_dimensions.x;
            const float world_x = (coords.x * chunk_dimensions.x + cell_x) * Globals::PARTICLE_SIZE;
            const float world_y = (coords.y * chunk_dimensions.y + cell_y) * Globals::PARTICLE_SIZE;

            gpu_cells[chunk_start_idx + i] = {
                glm::vec4(p.base_color.r, p.base_color.g, p.base_color.b, p.base_color.a),
                glm::vec4(p.color.r, p.color.g, p.color.b, p.color.a),
                glm::vec2(world_x, world_y),
                glm::uvec2(static_cast<std::uint32_t>(p.type), 0u)};
        }
    }

    // if (gpu_chunks.empty())
    // {
    //     // Debug: log if we culled everything
    //     static int empty_frame_count = 0;
    //     if (++empty_frame_count < 5)
    //     {
    //         std::cerr << "[WorldRenderer] No visible chunks! Culled: " << culled_chunks
    //                   << " / " << active_chunks->size() << "\n";
    //     }
    //     return;
    // }

    const int actual_chunk_count = static_cast<int>(gpu_chunks.size());
    const int actual_total_cells = static_cast<int>(gpu_cells.size());
    const int actual_total_vertices = actual_total_cells * vertices_per_cell;

    // Upload all cell data to GPU (binding 0)
    particle_ssbo->fill_with_data(gpu_cells, GL_STREAM);
    particle_ssbo->bind_base(0);

    // Upload chunk metadata to GPU (binding 1)
    chunk_ssbo->fill_with_data(gpu_chunks, GL_STREAM);
    chunk_ssbo->bind_base(1);

    // Allocate vertex output buffer (binding 2 for compute, binding 0 for render)
    const std::size_t vertex_buffer_size = actual_total_vertices * sizeof(GPUVertex);
    vertex_ssbo->allocate(vertex_buffer_size, GL_STREAM);
    vertex_ssbo->bind_base(2);

    // Reset and bind atomic counter (binding 3)
    std::uint32_t zero = 0;
    vertex_counter_ssbo->bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(std::uint32_t), &zero, GL_STREAM);
    vertex_counter_ssbo->bind_base(3);

    // Dispatch compute shader for ALL cells - GPU will skip empty ones
    compute_shader->use();
    compute_shader->set_float("particle_size", Globals::PARTICLE_SIZE);
    compute_shader->set_ivec2("chunk_dimensions", chunk_dimensions);
    compute_shader->set_int("cells_per_chunk", cells_per_chunk);
    compute_shader->set_int("chunk_count", actual_chunk_count);
    compute_shader->set_vec4("visible_bounds", glm::vec4(min_visible_x, min_visible_y, max_visible_x, max_visible_y));

    // Process all cells in 3D: XY for cell coords, Z for chunk index
    const int work_groups_x = (chunk_dimensions.x + 15) / 16;
    const int work_groups_y = (chunk_dimensions.y + 15) / 16;
    const int work_groups_z = actual_chunk_count;

    compute_shader->dispatch(work_groups_x, work_groups_y, work_groups_z);
    compute_shader->set_memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // Read back actual vertex count from atomic counter
    std::uint32_t actual_vertex_count = 0;
    vertex_counter_ssbo->bind();
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(std::uint32_t), &actual_vertex_count);

    // Rebind vertex SSBO to binding 0 for rendering
    vertex_ssbo->bind_base(0);

    // Render only actually written vertices
    render_shader->use();
    render_shader->set_mat4("view_projection", projection);
    dummy_VAO->bind();

    glDrawArrays(GL_TRIANGLES, 0, actual_vertex_count);

    // Cleanup bindings
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
    // glBindVertexArray(0);
}

void World_Renderer::clear_buffers()
{
    // vertices.clear();
    // indices.clear();
}

void World_Renderer::fill_vertices()
{
}

void World_Renderer::render_chunk_borders()
{
}

void World_Renderer::render_test_triangle()
{
    // shader->use();
    // shader->set_mat4("projection", projection);

    // std::vector<float> vertices = {
    //     // First triangle
    //     300.0f, 200.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left
    //     700.0f, 200.0f, 0.0f, 1.0f, 0.0f, 1.0f, // bottom right
    //     700.0f, 600.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right

    //     // Second triangle
    //     300.0f, 200.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left
    //     700.0f, 600.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
    //     300.0f, 600.0f, 0.0f, 1.0f, 0.0f, 1.0f  // top left
    // };

    // VAO->bind();
    // VBO->fill_with_data_vector(vertices, GL_DYNAMIC);

    // VAO->setup_vertex_attribute_pointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    // VAO->setup_vertex_attribute_pointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));

    // glDrawArrays(GL_TRIANGLES, 0, 6);
}

void World_Renderer::add_chunk_to_batch(Chunk *chunk)
{
    // auto [chunk_width, chunk_height] = world->get_chunk_dimensions();
    // auto coords = chunk->coords;

    // int world_x = coords.x * chunk_width * Globals::PARTICLE_SIZE;
    // int world_y = coords.y * chunk_height * Globals::PARTICLE_SIZE;

    // const auto *chunk_data = chunk->get_chunk_data();

    // unsigned int last = vertices.size();

    // // std::cout << coords.x << ';' << coords.y << '\n';

    // // vymysliet
    // for (int i = 0; i < chunk_height; i++)    // vyska
    //     for (int j = 0; j < chunk_width; j++) // sirka
    //     {
    //         // std::cout << i << ' ' << j << '\n';

    //         // auto cell = chunk->get_worldcell(j, i);

    //         int index = i * chunk_width + j;
    //         const WorldCell &cell = (*chunk_data)[index];

    //         const Particle &particle = cell.particle;

    //         if (particle.type == Particle_Type::EMPTY)
    //         {
    //             continue;
    //         }

    //         const Color &color = particle.color;

    //         unsigned int base = vertices.size();

    //         int offset_x = j * Globals::PARTICLE_SIZE;
    //         int offset_y = i * Globals::PARTICLE_SIZE;

    //         vertices.insert(vertices.end(), {
    //                                             Vertex(world_x + offset_x, world_y + offset_y, color),
    //                                             Vertex(world_x + offset_x + Globals::PARTICLE_SIZE, world_y + offset_y, color),
    //                                             Vertex(world_x + offset_x, world_y + offset_y + Globals::PARTICLE_SIZE, color),
    //                                             Vertex(world_x + offset_x + Globals::PARTICLE_SIZE, world_y + offset_y + Globals::PARTICLE_SIZE, color),
    //                                         });

    //         for (const auto indice : QUAD_INDICES)
    //         {
    //             indices.push_back(indice + base);
    //         }
    //     }
}

void World_Renderer::render_world()
{
    // clear_buffers();

    // auto active_chunks = world->get_active_chunks();

    // if (active_chunks->empty())
    //     return;

    // // Pre-allocate to avoid reallocations during rendering
    // size_t chunk_count = active_chunks->size();
    // vertices.reserve(chunk_count * 5000); // Assume ~50% filled chunks
    // indices.reserve(chunk_count * 7500);

    // // Render active chunks
    // for (const auto &active_coords : *active_chunks)
    // {
    //     auto it = world->get_chunks()->find(active_coords);
    //     if (it == world->get_chunks()->end())
    //         continue;

    //     auto chunk = it->second.get();
    //     add_chunk_to_batch(chunk);
    // }

    // if (vertices.empty())
    //     return;

    // shader->use();
    // shader->set_mat4("projection", projection);

    // VAO->bind();
    // VBO->fill_with_data_vector(vertices, GL_STREAM); // GL_STREAM for per-frame data
    // EBO->fill_with_data(indices, GL_STREAM);

    // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
