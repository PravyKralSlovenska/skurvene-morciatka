#include "engine/renderer/world_renderer.hpp"

#include "engine/renderer/buffers/vertex_buffer_object.hpp"
#include "engine/renderer/buffers/vertex_array_object.hpp"
#include "engine/renderer/buffers/element_array_object.hpp"
#include "engine/particle/particle.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/renderer/shader.hpp"
#include "others/GLOBALS.hpp"
#include "others/utils.hpp"

World_Renderer::World_Renderer(World *world)
    : world(world) {}

World_Renderer::~World_Renderer() = default;

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

    // chunk_VAO = std::make_unique<VERTEX_ARRAY_OBJECT>();
    // chunk_VAO->bind();

    // chunk_VBO = std::make_unique<VERTEX_BUFFER_OBJECT>();
    // chunk_VBO->bind();

    // chunk_EBO = std::make_unique<ELEMENT_ARRAY_BUFFER>();
    // chunk_EBO->bind();

    // // pre suradnice
    // chunk_VAO->setup_vertex_attribute_pointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    // // pre farbu
    // chunk_VAO->setup_vertex_attribute_pointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));

    // chunk_shader = std::make_unique<Shader>("../shaders/chunk_vertex.glsl", "../shaders/chunk_fragment.glsl");
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
    vertices.clear();
    indices.clear();
}

void World_Renderer::fill_vertices()
{
}

void World_Renderer::render_chunk_borders()
{
}

void World_Renderer::add_chunk_to_batch(Chunk *chunk)
{
    auto [chunk_width, chunk_height] = world->get_chunk_dimensions();
    auto coords = chunk->coords;

    int world_x = coords.x * chunk_width * Globals::PARTICLE_SIZE;
    int world_y = coords.y * chunk_height * Globals::PARTICLE_SIZE;

    unsigned int last = vertices.size();

    // std::cout << coords.x << ';' << coords.y << '\n';

    // vymysliet
    for (int i = 0; i < chunk_height; i++)    // vyska
        for (int j = 0; j < chunk_width; j++) // sirka
        {
            // std::cout << i << ' ' << j << '\n';

            auto cell = chunk->get_worldcell(j, i);

            Particle *particle = &cell->particle;

            if (particle->type == Particle_Type::EMPTY)
            {
                continue;
            }

            Color *color = &particle->color;

            unsigned int base = vertices.size();

            int offset_x = j * Globals::PARTICLE_SIZE;
            int offset_y = i * Globals::PARTICLE_SIZE;

            vertices.insert(vertices.end(), {
                                                Vertex(world_x + offset_x, world_y + offset_y, *color),
                                                Vertex(world_x + offset_x + Globals::PARTICLE_SIZE, world_y + offset_y, *color),
                                                Vertex(world_x + offset_x, world_y + offset_y + Globals::PARTICLE_SIZE, *color),
                                                Vertex(world_x + offset_x + Globals::PARTICLE_SIZE, world_y + offset_y + Globals::PARTICLE_SIZE, *color),
                                            });

            for (const auto indice : QUAD_INDICES)
            {
                indices.push_back(indice + base);
            }
        }
}

void World_Renderer::render_world()
{
    clear_buffers();

    // reserve verticies
    // reserve indicies

    // render_chunks();
    auto all_chunks = world->get_chunks();
    auto active_chunks = world->get_active_chunks();

    // len aktivne chunky
    for (const auto &active_coords : *active_chunks)
    {
        auto it = all_chunks->find(active_coords);
        if (it == all_chunks->end())
        {
            // nenasiel aktivny render chunk
            continue;
        }

        auto chunk = it->second.get();

        add_chunk_to_batch(chunk);
    }

    // vsetky chunky
    // for (const auto &entry : *all_chunks)
    // {
    //     if (!entry.second) continue;
    //     add_chunk_to_batch(entry.second.get());
    // }

    if (vertices.empty())
    {
        return;
    }

    // std::cout << "vertices:\t" << verticsudoes.size() << '\n';
    // std::cout << "indices: \t" << indices.size() << '\n';

    shader->use();
    shader->set_mat4("projection", projection);

    VAO->bind();
    VBO->fill_with_data_vector(vertices, GL_DYNAMIC);
    EBO->fill_with_data(indices, GL_DYNAMIC);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
