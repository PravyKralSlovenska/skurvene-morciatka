#include "engine/renderer/world_renderer.hpp"

World_Renderer::World_Renderer(World *world)
    : world(world) {}

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
    // for (const auto &cell : world->get_world_curr())
    // {
    //     if (cell.particle.type == Particle_Type::EMPTY)
    //     {
    //         continue;
    //     }

    //     auto x = cell.coords.x * world->scale;
    //     auto y = cell.coords.y * world->scale;

    //     unsigned int last = vertices.size();

        // vertices.emplace_back(x, y, cell.particle.color);
        // vertices.emplace_back(x + world->scale, y, cell.particle.color);
        // vertices.emplace_back(x + world->scale, y + world->scale, cell.particle.color);
        // vertices.emplace_back(x, y + world->scale, cell.particle.color);

        // indices.push_back(last);
        // indices.push_back(last + 1);
        // indices.push_back(last + 2);
        // indices.push_back(last);
        // indices.push_back(last + 2);
        // indices.push_back(last + 3);
    // }
}

void World_Renderer::render_chunks()
{
    // active chunks will be green
    // all other chunks will be red

    // auto chunks = world->get_chunks();

    // for (auto i = chunks->begin(); i != chunks->end(); i++)
    // {
    //     // chunk_vertices.push_back(chunks[i);
    //     glm::ivec2 coords = i->first;
    //     Chunk chunk = *i->second;

    //     float r = 1.0f, g = 0.0f, b = 0.0f, a = 1.0f;
    //     if (Chunk_States state = chunk.get_state(); state == Chunk_States::LOADED)
    //     {
    //         r = 0.0f;
    //         g = 1.0f;
    //     }

    //     int x = coords.x * chunk.width;
    //     int y = coords.y * chunk.height;
    // }

    // chunk_shader->use();
    // chunk_shader->set_mat4("projection", projection);

    // chunk_VAO->bind();

    // chunk_VBO->fill_with_data_vector(chunk_vertices, GL_DYNAMIC);
    // chunk_EBO->fill_with_data(chunk_indices, GL_DYNAMIC);

    // glDrawElements(GL_TRIANGLES, chunk_indices.size(), GL_UNSIGNED_INT, 0);

    // chunk_vertices.clear();
}

void World_Renderer::add_chunk_to_batch(Chunk *chunk)
{
    auto [chunk_width, chunk_height] = world->get_chunk_dimensions();
    auto coords = chunk->coords;

    int world_x = coords.x * chunk_width;
    int world_y = coords.y * chunk_height;

    unsigned int last = vertices.size();

    int size_of_a_particle = 10;

    // vymysliet
    for (int i = 0; i < chunk_height; i++) // vyska
    for (int j = 0; j < chunk_width; j++)  // sirka
    {
        auto cell = chunk->get_worldcell(j, i);
        
        Particle *particle = &cell->particle;
        Color *color = &particle->color;

        unsigned int base = vertices.size();

        int offset_x = j * size_of_a_particle;
        int offset_y = i * size_of_a_particle;

        vertices.insert(vertices.end(), {
            Vertex(world_x                     , world_y                     , *color),
            Vertex(world_x + size_of_a_particle, world_y                     , *color),
            Vertex(world_x                     , world_y + size_of_a_particle, *color),
            Vertex(world_x + size_of_a_particle, world_y + size_of_a_particle, *color),
        });

        for (const auto indice : QUAD_INDICES)
        {
            indices.push_back(indice + base);
        }
    }
}

void World_Renderer::render_world()
{
    auto all_chunks = world->get_chunks();
    auto active_chunks = world->get_active_chunks();

    for (const auto &active_coords : *active_chunks)
    {
        auto it = all_chunks->find(active_coords);
        if (it == all_chunks->end())
        {
            // nenasiel activny render chunk
            continue;
        }

        auto chunk = it->second.get();

        add_chunk_to_batch(chunk);
    }

    // render_chunks();

    // fill_vertices();

    // if (vertices.empty())
    // {
    //     clear_buffers();
    //     return;
    // }

    // std::cout << "vertices:\t" << verticsudoes.size() << '\n';
    // std::cout << "indices: \t" << indices.size() << '\n';

    shader->use();
    shader->set_mat4("projection", projection);

    VAO->bind();
    VBO->fill_with_data_vector(vertices, GL_DYNAMIC);
    EBO->fill_with_data(indices, GL_DYNAMIC);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
