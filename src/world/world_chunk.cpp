#include "engine/world/world_chunk.hpp"

Chunk::Chunk(glm::ivec2 coords, int width, int height)
    : coords(coords), width(width), height(height)
{
    init_chunk_data();
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

    chunk_data.shrink_to_fit();
}

inline int Chunk::get_index(int x, int y)
{
    return y * width + x;
}

void Chunk::move_cell()
{
    
}

Chunk_States Chunk::get_state()
{
    return state;
}

void Chunk::set_state(Chunk_States state)
{
    this->state = state;
}

bool Chunk::is_empty(int x, int y)
{
    return is_empty(get_index(x, y));
}

bool Chunk::is_empty(int index)
{
    return chunk_data[index].particle.type == Particle_Type::EMPTY;
}

void Chunk::set_worldcell(int x, int y, Particle *particle)
{
    set_worldcell(get_index(x, y), particle);
}

void Chunk::set_worldcell(int index, Particle *particle)
{
    chunk_data[index].set_particle(*particle);
}

WorldCell* Chunk::get_worldcell(int x, int y)
{
    return get_worldcell(get_index(x, y));
}

WorldCell* Chunk::get_worldcell(int index)
{
    return &chunk_data[index];
}
