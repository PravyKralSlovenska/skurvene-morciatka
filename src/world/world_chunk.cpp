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
            chunk_data.emplace_back(glm::ivec2(y, x));
            // chunk_data.emplace_back(glm::ivec2(x, y), create_sand());
        }
    }

    // chunk_data.shrink_to_fit();
}

inline int Chunk::get_index(int x, int y)
{
    return y * width + x;
}

void Chunk::move_cell()
{
}

void Chunk::make_cached_verticies()
{
}

void Chunk::make_cached_indicies()
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

bool Chunk::is_dirty()
{
    // return is_dirty;
}

bool Chunk::is_empty(int x, int y)
{
    return is_empty(get_index(x, y));
}

bool Chunk::is_empty(int index)
{
    return chunk_data[index].particle.type == Particle_Type::EMPTY;
}

void Chunk::set_worldcell(const glm::ivec2 &coords, Particle_Type particle)
{
    set_worldcell(get_index(coords.x, coords.y), particle);
}

void Chunk::set_worldcell(int x, int y, Particle_Type particle)
{
    set_worldcell(get_index(x, y), particle);
}

void Chunk::set_worldcell(int index, Particle_Type particle)
{
    switch (particle)
    {
    case Particle_Type::EMPTY:
        break;

    case Particle_Type::SAND:
        chunk_data[index].set_particle(create_sand());
        break;

    case Particle_Type::WATER:
        chunk_data[index].set_particle(create_water());
        break;

    case Particle_Type::SMOKE:
        chunk_data[index].set_particle(create_smoke());
        break;

    case Particle_Type::STONE:
        chunk_data[index].set_particle(create_stone());
        break;

    default:
        break;
    }
}

WorldCell *Chunk::get_worldcell(int x, int y)
{
    return get_worldcell(get_index(x, y));
}

WorldCell *Chunk::get_worldcell(int index)
{
    return &chunk_data[index];
}

std::vector<Vertex> *Chunk::get_cached_verticies()
{
    return &cached_verticies;
}

std::vector<unsigned int> *Chunk::get_cached_indicies()
{
    return &cached_indicies;
}
