#include <iostream>
#include "others/utils.hpp"
#include "engine/world.hpp"

/*
 * WORLD CELL
 */
Vertex::Vertex() { }
Vertex::Vertex(float x, float y, float r, float g, float b, float a) : x(x), y(y), r(r), g(g), b(b), a(a) {}

/*
 * WORLD CELL
 */
WorldCell::WorldCell() {}
WorldCell::WorldCell(int x, int y) : x(x), y(y) {}

void WorldCell::set_empty()
{
    this->empty = true;
    this->particle;
}

void WorldCell::set_particle(Particle p)
{
    this->empty = false;
    this->particle = p;
}

std::string WorldCell::get_info()
{
    std::string return_value = std::to_string(x) + ";" + std::to_string(y) + ";" + particle.name;
    return return_value;
}

/*
 * WORLD
 */
World::World(int w, int h, int ps) : width(w), height(h), particle_size(ps)
{
    seed = 1;

    rows = height / particle_size;
    cols = width / particle_size;

    vertex_buffer.reserve(rows * cols * 4);
    indices.reserve(rows * cols * 6);

    world_curr.reserve(rows * cols);
    world_next.reserve(rows * cols);

    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            world_curr.emplace_back(x, y);
            world_next.emplace_back(x, y);
        }
    }

}

void World::add_particle(Particle particle, int x, int y)
{
    if (in_world_range(x, y, rows, cols))
    {
        world_curr[y * cols + x].set_particle(particle);
    }
}

void World::update_world()
{
    // world_curr world_next
    for (int riadok = rows - 1; riadok >= 0; --riadok)
    {
        for (int stlpec = cols - 1; stlpec >= 0; --stlpec)
        {
            WorldCell worldcell = world_curr[riadok * cols + stlpec];
            if (!worldcell.empty)
            {
                push_vertex_buffer(worldcell.particle, stlpec, riadok);
            }
        }
    }
}

void World::push_vertex_buffer(Particle &particle, int grid_x, int grid_y)
{
    float x = grid_x * particle_size;
    float y = grid_y * particle_size;

    float r = particle.color[0] / 255;
    float g = particle.color[1] / 255;
    float b = particle.color[2] / 255;
    float a = particle.opacity; // ALPHA

    int base = vertex_buffer.size();

    vertex_buffer.emplace_back(x,                 y,                 r, g, b, a);
    vertex_buffer.emplace_back(x + particle_size, y,                 r, g, b, a);
    vertex_buffer.emplace_back(x,                 y + particle_size, r, g, b, a);
    vertex_buffer.emplace_back(x + particle_size, y + particle_size, r, g, b, a);
    
    // indicies = 0, 1, 3, 0, 2, 3
    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 3);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
}

void World::clear_vertex_buffer()
{
    vertex_buffer.clear();
    indices.clear();

    vertex_buffer.reserve(rows * cols * 4);
    indices.reserve(rows * cols * 6);
}

void World::swap_worlds()
{
    std::swap(world_curr, world_next);
    world_next.clear();
    world_next.resize(rows * cols);
}

void World::clear_worlds()
{
    world_curr.clear();
    world_next.clear();
}

void World::render_world()
{
}

void World::load_chunk()
{
}

void World::unload_chunk()
{
}

// testy a debug
void World::print_world_info()
{
    for (WorldCell worldcell : world_curr)
    {
        std::cout << worldcell.get_info() << ' ';
        if (worldcell.x == cols - 1)
        {
            std::cout << '\n';
        }
    }
    std::cout << "-----------------------------------------------" << '\n';
}