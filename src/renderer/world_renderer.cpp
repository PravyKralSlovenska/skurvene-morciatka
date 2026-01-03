#include "engine/renderer/world_renderer.hpp"

#include "engine/renderer/buffers/vertex_buffer_object.hpp"
#include "engine/renderer/buffers/vertex_array_object.hpp"
#include "engine/renderer/buffers/element_array_object.hpp"
#include "engine/renderer/buffers/shader_storage_buffer_object.hpp"
#include "engine/particle/particle.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/world/world_cell_gpu.hpp"
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
    if (!world)
        return;

    const auto *active_chunks = world->get_active_chunks();
    if (!active_chunks || active_chunks->empty())
        return;

    const glm::ivec2 chunk_dimensions = world->get_chunk_dimensions();
    const int cells_per_chunk = chunk_dimensions.x * chunk_dimensions.y;

    struct GPUChunkInfo
    {
        glm::ivec2 world_coords;
        int cell_data_offset;
        int cell_count;
    };

    struct GPUVertex
    {
        glm::vec2 position;
        glm::vec2 _padding;
        glm::vec4 color;
    };

    // Calculate visible bounds
    glm::mat4 inv_projection = glm::inverse(projection);
    glm::vec4 ndc_corners[4] = {
        glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
        glm::vec4(1.0f, -1.0f, 0.0f, 1.0f),
        glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)};

    float min_x = FLT_MAX, max_x = -FLT_MAX;
    float min_y = FLT_MAX, max_y = -FLT_MAX;

    for (int i = 0; i < 4; ++i)
    {
        glm::vec4 world_pos = inv_projection * ndc_corners[i];
        world_pos /= world_pos.w;
        min_x = std::min(min_x, world_pos.x);
        max_x = std::max(max_x, world_pos.x);
        min_y = std::min(min_y, world_pos.y);
        max_y = std::max(max_y, world_pos.y);
    }

    const float margin = Globals::PARTICLE_SIZE * 10.0f;
    min_x -= margin;
    max_x += margin;
    min_y -= margin;
    max_y += margin;

    const float chunk_pixel_width = chunk_dimensions.x * Globals::PARTICLE_SIZE;
    const float chunk_pixel_height = chunk_dimensions.y * Globals::PARTICLE_SIZE;

    // Collect visible chunks
    std::vector<GPUChunkInfo> gpu_chunks;
    std::vector<Chunk *> visible_chunks;

    gpu_chunks.reserve(active_chunks->size());
    visible_chunks.reserve(active_chunks->size());

    for (const auto &coords : *active_chunks)
    {
        float chunk_min_x = coords.x * chunk_pixel_width;
        float chunk_max_x = chunk_min_x + chunk_pixel_width;
        float chunk_min_y = coords.y * chunk_pixel_height;
        float chunk_max_y = chunk_min_y + chunk_pixel_height;

        if (chunk_max_x < min_x || chunk_min_x > max_x ||
            chunk_max_y < min_y || chunk_min_y > max_y)
        {
            continue;
        }

        Chunk *chunk = world->get_chunk(coords);
        if (!chunk)
            continue;

        const auto *chunk_data = chunk->get_chunk_data();
        if (!chunk_data || chunk_data->empty())
            continue;

        visible_chunks.push_back(chunk);
        gpu_chunks.push_back({coords, 0, cells_per_chunk});
    }

    if (gpu_chunks.empty())
        return;

    // Calculate offsets
    int total_cells = 0;
    for (size_t i = 0; i < visible_chunks.size(); ++i)
    {
        gpu_chunks[i].cell_data_offset = total_cells;
        total_cells += cells_per_chunk;
    }

    const int chunk_count = static_cast<int>(gpu_chunks.size());
    const int max_vertices = total_cells * 6;

    particle_ssbo->bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, total_cells * sizeof(GPUWorldCell), nullptr, GL_STREAM);

    for (size_t chunk_index = 0; chunk_index < visible_chunks.size(); ++chunk_index)
    {
        const auto &chunk_gpu_data = visible_chunks[chunk_index]->get_gpu_chunk_data();

        if (chunk_gpu_data.empty())
        {
            continue;
        }

        const std::size_t offset_bytes = static_cast<std::size_t>(gpu_chunks[chunk_index].cell_data_offset) * sizeof(GPUWorldCell);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                        offset_bytes,
                        chunk_gpu_data.size() * sizeof(GPUWorldCell),
                        chunk_gpu_data.data());
    }

    particle_ssbo->bind_base(0);

    // Upload chunk metadata
    chunk_ssbo->fill_with_data(gpu_chunks, GL_STREAM);
    chunk_ssbo->bind_base(1);

    // Allocate vertex output
    vertex_ssbo->allocate(max_vertices * sizeof(GPUVertex), GL_STREAM);
    vertex_ssbo->bind_base(2);

    // Reset counter
    uint32_t zero = 0;
    vertex_counter_ssbo->bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t), &zero, GL_STREAM);
    vertex_counter_ssbo->bind_base(3);

    // Dispatch
    compute_shader->use();
    compute_shader->set_float("particle_size", Globals::PARTICLE_SIZE);
    compute_shader->set_ivec2("chunk_dimensions", chunk_dimensions);
    compute_shader->set_int("cells_per_chunk", cells_per_chunk);
    compute_shader->set_int("chunk_count", chunk_count);
    compute_shader->set_vec4("visible_bounds", glm::vec4(min_x, min_y, max_x, max_y));

    const int work_groups_x = (chunk_dimensions.x + 15) / 16;
    const int work_groups_y = (chunk_dimensions.y + 15) / 16;
    const int work_groups_z = chunk_count;

    compute_shader->dispatch(work_groups_x, work_groups_y, work_groups_z);
    compute_shader->set_memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // Read vertex count
    uint32_t actual_vertex_count = 0;
    vertex_counter_ssbo->bind();
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &actual_vertex_count);

    // Render
    vertex_ssbo->bind_base(0);
    render_shader->use();
    render_shader->set_mat4("view_projection", projection);
    dummy_VAO->bind();

    glDrawArrays(GL_TRIANGLES, 0, actual_vertex_count);
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
