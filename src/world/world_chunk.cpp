#include "engine/world/world_chunk.hpp"

#include <iostream>
#include <cstdint>

#include "engine/world/world_cell.hpp"
#include "engine/particle/particle.hpp"
#include "others/utils.hpp"

Chunk::Chunk(glm::ivec2 coords, int width, int height)
    : coords(coords), width(width), height(height)
{
    // init_chunk_data();
}

void Chunk::init_chunk_data()
{
    chunk_data.reserve(width * height);

    for (int y{0}; y < height; y++)
    {
        for (int x{0}; x < width; x++)
        {
            chunk_data.emplace_back(glm::ivec2(x, y));
            // chunk_data.emplace_back(glm::ivec2(x, y), create_stone());
        }
    }

    chunk_data.shrink_to_fit();
}

glm::ivec2 Chunk::get_chunk_dimensions()
{
    return glm::ivec2(width, height);
}

inline int Chunk::get_index(int x, int y)
{
    return y * width + x;
}

void Chunk::move_worldcell(WorldCell &from, WorldCell &to)
{
    std::swap(from.particle, to.particle);
    gpu_dirty = true;
}

void Chunk::make_cached_verticies()
{
}

void Chunk::make_cached_indicies()
{
}

std::vector<WorldCell> *Chunk::get_chunk_data()
{
    return &chunk_data;
}

void Chunk::set_chunk_data(std::vector<WorldCell> &new_chunk_data)
{
    if (new_chunk_data.size() != width * height)
    {
        // std::cerr << "chunk data nemaju dobry rozmer. potrebujem rozmer: " << width * height << " ale prisiel rozmer: " << new_chunk_data.size() << '\n';
        return;
    }

    chunk_data = new_chunk_data;
    gpu_dirty = true;
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
    return get_worldcell(index)->particle.type == Particle_Type::EMPTY;
}

WorldCell *Chunk::get_if_not_empty(const int x, const int y)
{
    return get_if_not_empty(get_index(x, y));
}

WorldCell *Chunk::get_if_not_empty(const int index)
{
    auto cell = get_worldcell(index);

    if (cell->particle.type != Particle_Type::EMPTY)
    {
        return cell;
    }

    return nullptr;
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

    gpu_dirty = true;
}

WorldCell *Chunk::get_worldcell(int x, int y)
{
    return get_worldcell(get_index(x, y));
}

WorldCell *Chunk::get_worldcell(int index)
{
    return &chunk_data[index];
}

void Chunk::rebuild_gpu_chunk_data()
{
    if (chunk_data.empty())
    {
        gpu_chunk_data.clear();
        gpu_dirty = false;
        return;
    }

    gpu_chunk_data.resize(chunk_data.size());

    for (std::size_t i = 0; i < chunk_data.size(); ++i)
    {
        const WorldCell &cell = chunk_data[i];
        const Particle &particle = cell.particle;

        GPUWorldCell &gpu_cell = gpu_chunk_data[i];
        gpu_cell.coords = cell.coords;
        gpu_cell.base_color = glm::vec4(particle.base_color.r, particle.base_color.g, particle.base_color.b, particle.base_color.a);
        gpu_cell.color = glm::vec4(particle.color.r, particle.color.g, particle.color.b, particle.color.a);
        gpu_cell.meta = glm::uvec4(static_cast<std::uint32_t>(particle.type),
                                   static_cast<std::uint32_t>(particle.state),
                                   static_cast<std::uint32_t>(particle.move),
                                   cell.visited ? 1u : 0u);
    }

    gpu_dirty = false;
}

const std::vector<GPUWorldCell> &Chunk::get_gpu_chunk_data()
{
    if (gpu_dirty)
    {
        rebuild_gpu_chunk_data();
    }

    return gpu_chunk_data;
}

std::vector<Vertex> *Chunk::get_cached_verticies()
{
    return &cached_verticies;
}

std::vector<unsigned int> *Chunk::get_cached_indicies()
{
    return &cached_indicies;
}
