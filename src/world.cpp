#include <iostream>

#include "../include/engine/world.hpp"
#include "../include/engine/particles.hpp"


void World::update_world()
{

}

void World::render_world()
{

}

void World::swap_worlds()
{
    std::swap(world, world_next);
}