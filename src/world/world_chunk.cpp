#include "engine/world/world_chunk.hpp"

Chunk::Chunk(glm::ivec2 coords)
    : coords(coords)
{
}

Chunk::Chunk(glm::ivec2 coords, int width, int height)
    : coords(coords), width(width), height(height)
{
    init_chunk_data();
}

Chunk::~Chunk() 
{
    
}

void Chunk::init_chunk_data()
{
    chunk_data.reserve(width * height);
    
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            chunk_data.emplace_back(glm::ivec2(x, y));
        }
    }
}