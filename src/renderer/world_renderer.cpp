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

World_Renderer::World_Renderer(World *world)
    : world(world) {}

World_Renderer::~World_Renderer()
{
}

void World_Renderer::init()
{
    VAO = std::make_unique<VERTEX_ARRAY_OBJECT>();
    VAO->bind();
    VAO->setup_vertex_attribute_pointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);                       // coords
    VAO->setup_vertex_attribute_pointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color)); // farba

    VBO = std::make_unique<VERTEX_BUFFER_OBJECT>();

    render_shader = std::make_unique<Shader>("../shaders/vertex.glsl", "../shaders/fragment.glsl");

    compute_shader = std::make_unique<Compute_Shader>("../shaders/compute_shader.glsl");
    compute_shader->use();
    compute_shader->dispatch(16, 16);
    compute_shader->set_memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    compute_shader->set_float("particle_size", Globals::PARTICLE_SIZE);
    compute_shader->set_ivec2("chunk_dimensions", world->get_chunk_dimensions());

    chunk_ssbo = std::make_unique<Shader_Storage_Buffer_Object>();
    particle_ssbo = std::make_unique<Shader_Storage_Buffer_Object>();
    vertex_ssbo = std::make_unique<Shader_Storage_Buffer_Object>();
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
    // if (!world) // je toto potrebne? kedze davam world ako povinny parameter do konstruktora
    // {
    //     std::cerr << "nie je pripojeny ziadny world\n";
    //     return;
    // }

    // 1. SPRACOVANIE
    auto active_chunks = world->get_active_chunks();

    if (active_chunks->empty())
    {
        return;
    }

    compute_shader->use();

    particle_ssbo->bind_base(0);
    vertex_ssbo->bind_base(1);

    for (const auto &coords : *active_chunks)
    {
        const auto chunk = world->get_chunk(coords);
        if (!chunk)
        {
            continue;
        }

        auto offset = glm::vec2(coords.x * chunk->width * Globals::PARTICLE_SIZE,
                                coords.y * chunk->height * Globals::PARTICLE_SIZE);

        compute_shader->set_vec2("chunk_world_offset", offset);
    }

    // 2. RENDEROVANIE
    render_shader->use();
    render_shader->set_mat4("projection", projection);

    for (const auto &coords : *active_chunks)
    {
        // zapln vao

        // size_t vertex_count = gpu_data.particle_count * 4;
        // size_t vertex_bytes = vertex_count * sizeof(Vertex);

        // // Bind the chunk's vertex SSBO
        // gpu_data.ssbo_vertices->bind();

        // // Copy from SSBO to VBO using glCopyBufferSubData
        // glBindBuffer(GL_COPY_READ_BUFFER, gpu_data.ssbo_vertices->id);
        // glBindBuffer(GL_COPY_WRITE_BUFFER, unified_vbo->id);

        // glCopyBufferSubData(
        //     GL_COPY_READ_BUFFER,
        //     GL_COPY_WRITE_BUFFER,
        //     0,                  // Read from start of SSBO
        //     current_offset,     // Write to current offset in VBO
        //     vertex_bytes
        // );

        // gpu_data.vertex_offset = current_offset;
        // current_offset += vertex_bytes;
    }

    // bind vao

    // glDrawElements(GL_TRIANGLES, total_indices, GL_UNSIGNED_INT, 0);
}

void World_Renderer::render_world_compute()
{
    // if (!world)
    // {
    //     std::cerr << "nie je pripojeny ziadny world\n";
    //     return;
    // }

    // auto active_chunks = world->get_active_chunks();
    // if (active_chunks->empty())
    // {
    //     return;
    // }

    // auto chunk_dimensions = world->get_chunk_dimensions();
    // int chunk_width = chunk_dimensions.x;
    // int chunk_height = chunk_dimensions.y;
    // const int particles_per_chunk = chunk_width * chunk_height;

    // // ==========================================
    // // PHASE 1: RUN COMPUTE SHADER ON ALL CHUNKS
    // // ==========================================
    // compute_shader->use();

    // // Calculate total size - 6 vertices per particle now!
    // size_t total_particles = active_chunks->size() * particles_per_chunk;
    // size_t total_vertices = total_particles * 6;                    // ✅ 6 instead of 4!
    // size_t total_vertex_bytes = total_vertices * 6 * sizeof(float); // vec2 + vec4

    // // Allocate unified vertex buffer
    // VBO->bind();
    // glBufferData(GL_ARRAY_BUFFER, total_vertex_bytes, nullptr, GL_STREAM);

    // size_t current_vertex_offset = 0;

    // for (const auto &coords : *active_chunks)
    // {
    //     const auto chunk = world->get_chunk(coords);
    //     if (!chunk)
    //         continue;

    //     const auto *chunk_data = chunk->get_chunk_data();
    //     if (!chunk_data)
    //         continue;

    //     // Prepare GPU particle data
    //     struct GPU_Particle
    //     {
    //         uint32_t type;
    //         uint32_t state;
    //         uint32_t _pad1;
    //         uint32_t _pad2;
    //         float color[4];
    //     };

    //     std::vector<GPU_Particle> gpu_particles;
    //     gpu_particles.reserve(particles_per_chunk);

    //     for (const auto &cell : *chunk_data)
    //     {
    //         const Particle &p = cell.particle;
    //         gpu_particles.push_back({static_cast<uint32_t>(p.type),
    //                                  static_cast<uint32_t>(p.state),
    //                                  0,
    //                                  0,
    //                                  {p.color.r, p.color.g, p.color.b, p.color.a}});
    //     }

    //     // Upload particle data
    //     particle_ssbo->fill_with_data(gpu_particles, GL_DYNAMIC);
    //     particle_ssbo->bind_base(0);

    //     // Allocate vertex output buffer - 6 vertices per particle!
    //     size_t chunk_vertex_bytes = particles_per_chunk * 6 * 6 * sizeof(float);
    //     vertex_ssbo->allocate(chunk_vertex_bytes, GL_DYNAMIC);
    //     vertex_ssbo->bind_base(1);

    //     // Set uniforms
    //     glm::vec2 chunk_world_offset(
    //         coords.x * chunk_width * Globals::PARTICLE_SIZE,
    //         coords.y * chunk_height * Globals::PARTICLE_SIZE);

    //     compute_shader->set_vec2("chunk_world_offset", chunk_world_offset);
    //     compute_shader->set_float("particle_size", Globals::PARTICLE_SIZE);
    //     compute_shader->set_ivec2("chunk_dimensions", chunk_width, chunk_height);

    //     // Dispatch compute shader
    //     int work_groups_x = (chunk_width + 15) / 16;
    //     int work_groups_y = (chunk_height + 15) / 16;
    //     compute_shader->dispatch(work_groups_x, work_groups_y, 1);

    //     // Wait for compute shader
    //     glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    //     // Copy vertices to unified VBO
    //     glBindBuffer(GL_COPY_READ_BUFFER, vertex_ssbo->id);
    //     glBindBuffer(GL_COPY_WRITE_BUFFER, VBO->id);

    //     glCopyBufferSubData(
    //         GL_COPY_READ_BUFFER,
    //         GL_COPY_WRITE_BUFFER,
    //         0,
    //         current_vertex_offset,
    //         chunk_vertex_bytes);

    //     current_vertex_offset += chunk_vertex_bytes;
    // }

    // // ==========================================
    // // PHASE 2: RENDER ALL CHUNKS IN ONE CALL
    // // ==========================================
    // render_shader->use();
    // render_shader->set_mat4("projection", projection);

    // VAO->bind();

    // // ✅ Just draw arrays - no indices needed!
    // glDrawArrays(GL_TRIANGLES, 0, total_vertices);

    // glBindVertexArray(0);
}

void World_Renderer::upload_chunk_to_gpu(const glm::ivec2 &chunk_coords, Chunk *chunk)
{
}

void World_Renderer::cleanup_chunk_gpu_data(const glm::ivec2 &chunk_coords)
{
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


// #version 430 core

// layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// struct Particle {
//     uint type;
//     uint state;
//     uint _pad1;
//     uint _pad2;
//     vec4 color;
// };

// struct Vertex {
//     vec2 position;
//     vec4 color;
// };

// layout(std430, binding = 0) readonly buffer ParticleBuffer {
//     Particle particles[];
// };

// layout(std430, binding = 1) writeonly buffer VertexBuffer {
//     Vertex vertices[];
// };

// uniform vec2 chunk_world_offset;
// uniform float particle_size;
// uniform ivec2 chunk_dimensions;

// void main() {
//     ivec2 cell_coords = ivec2(gl_GlobalInvocationID.xy);
    
//     if (cell_coords.x >= chunk_dimensions.x || 
//         cell_coords.y >= chunk_dimensions.y) {
//         return;
//     }
    
//     int particle_index = cell_coords.y * chunk_dimensions.x + cell_coords.x;
//     Particle particle = particles[particle_index];
    
//     if (particle.type == 0) {
//         return;
//     }
    
//     vec2 world_pos = chunk_world_offset + vec2(cell_coords) * particle_size;
    
//     // Generate 6 vertices (2 triangles) per particle
//     int vertex_base = particle_index * 6;
    
//     vec2 p0 = world_pos;                                    // top-left
//     vec2 p1 = world_pos + vec2(particle_size, 0.0);        // top-right
//     vec2 p2 = world_pos + vec2(0.0, particle_size);        // bottom-left
//     vec2 p3 = world_pos + vec2(particle_size, particle_size); // bottom-right
    
//     // First triangle: 0, 1, 2
//     vertices[vertex_base + 0] = Vertex(p0, particle.color);
//     vertices[vertex_base + 1] = Vertex(p1, particle.color);
//     vertices[vertex_base + 2] = Vertex(p2, particle.color);
    
//     // Second triangle: 1, 3, 2
//     vertices[vertex_base + 3] = Vertex(p1, particle.color);
//     vertices[vertex_base + 4] = Vertex(p3, particle.color);
//     vertices[vertex_base + 5] = Vertex(p2, particle.color);
// }