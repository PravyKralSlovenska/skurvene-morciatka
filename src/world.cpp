#include <iostream>
#include "GLOBALS.hpp"
#include "engine/world.hpp"

void WorldCell::set_empty()
{
    this->empty = true;
    this->particle = nullptr;
}

void WorldCell::set_particle(std::shared_ptr<Particle> p)
{
    this->empty = false;
    this->particle = p;
}

void World::add_particle(int particle, int x, int y)
{
    // world_curr[y][x] = particle;
}

void World::update_world()
{
    for (size_t i = world_curr.size(); i-- > 0;)
    {
        for (size_t j = world_curr[i].size(); j-- > 0;)
        {
            // std::pair<int, int> coords = world_curr[i][j].getCoords();
            // world_next[coords.second][coords.first] = world_curr[i][j];
        }
    }
}

void World::swap_worlds()
{
    std::swap(world_curr, world_next);
    world_next.clear();
    world_next.resize(world_curr.size(), std::vector<WorldCell>(world_curr[0].size()));
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
void World::print_world()
{
    for (auto riadok : world_curr)
    {
        for (auto element : riadok)
        {
            // std::cout << element << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "--------------------\n";
}