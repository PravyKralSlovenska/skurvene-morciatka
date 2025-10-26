#include "engine/world/world.hpp"

World::World() {
}

// World::~World() {}

int World::get_index(int x, int y) 
{
    return y * width + x;
}

Chunk World::create_chunk()
{
    
}

void World::add_chunk(Chunk chunk)
{

}

void World::remove_chunk(int index)
{

}

Chunk& World::get_chunk(int x, int y) 
{
    // return *world[get_index(x, y)];
}

Chunk& World::get_chunk(int index) 
{
    // return *world[index];
}