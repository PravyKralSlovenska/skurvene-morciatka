#include "engine/renderer/world_renderer.hpp"

#include "engine/renderer/buffers/vertex_buffer_object.hpp"
#include "engine/renderer/buffers/vertex_array_object.hpp"
#include "engine/renderer/buffers/element_array_object.hpp"
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
    for (auto &[coords, gpu_data] : gpu_chunks)
    {
        glDeleteBuffers(1, &gpu_data.ssbo_particles);
        glDeleteBuffers(1, &gpu_data.ssbo_vertices);
        glDeleteVertexArrays(1, &gpu_data.vao);
    }
}

// World_Renderer::~World_Renderer() {}

void World_Renderer::init()
{
    VAO = std::make_unique<VERTEX_ARRAY_OBJECT>();
    VAO->bind();
    // VAO->setup_vertex_attribute_pointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    VBO = std::make_unique<VERTEX_BUFFER_OBJECT>();
    VBO->bind();
    // VBO->fill_with_data_raw(GL_DYNAMIC)

    EBO = std::make_unique<ELEMENT_ARRAY_BUFFER>();
    EBO->bind();

    // pre suradnice
    VAO->setup_vertex_attribute_pointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // pre farbu
    VAO->setup_vertex_attribute_pointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    shader = std::make_unique<Shader>("../shaders/vertex.glsl", "../shaders/fragment.glsl");

    compute_shader = std::make_unique<Compute_Shader>("../shaders/compute_shader.glsl");
}

void World_Renderer::upload_chunk_to_gpu(const glm::ivec2 &chunk_coords, Chunk *chunk)
{
}

void World_Renderer::cleanup_chunk_gpu_data(const glm::ivec2 &chunk_coords)
{
}

void World_Renderer::render_world_compute()
{
}

void World_Renderer::set_world(World *world)
{
    this->world = world;
}

void World_Renderer::set_projection(glm::mat4 projection)
{
    this->projection = projection;
}

void World_Renderer::render_test_triangle()
{
    shader->use();
    shader->set_mat4("projection", projection);

    std::vector<float> vertices = {
        // First triangle
        300.0f, 200.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left
        700.0f, 200.0f, 0.0f, 1.0f, 0.0f, 1.0f, // bottom right
        700.0f, 600.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right

        // Second triangle
        300.0f, 200.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left
        700.0f, 600.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
        300.0f, 600.0f, 0.0f, 1.0f, 0.0f, 1.0f  // top left
    };

    VAO->bind();
    VBO->fill_with_data_vector(vertices, GL_DYNAMIC);

    VAO->setup_vertex_attribute_pointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    VAO->setup_vertex_attribute_pointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6);
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
