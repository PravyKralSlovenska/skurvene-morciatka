#include "engine/world/world_ca_generation.hpp"

#include <iostream>

#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/particle/particle.hpp"
#include "engine/world/world_biomes.hpp"
#include "others/utils.hpp"

World_CA_Generation::World_CA_Generation(const int chunk_width, const int chunk_height)
{
    seed = Random_Machine::get_int_from_range(0, 9999);

    recalculate_noises();
    setup_biome_noises();

    default_chunk_data.reserve(chunk_width * chunk_height);
    for (int y{0}; y < chunk_height; y++)
    {
        for (int x{0}; x < chunk_width; x++)
        {
            default_chunk_data.emplace_back(glm::ivec2(x, y), create_stone());
        }
    }
}

void World_CA_Generation::recalculate_noises()
{
    cave_noise.SetSeed(seed);
    cave_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    cave_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    cave_noise.SetFractalOctaves(4);
    cave_noise.SetFrequency(0.02f);

    biome_noise.SetSeed(seed);
    biome_noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    biome_noise.SetFrequency(0.002f);
}

glm::ivec2 World_CA_Generation::calculate_world_coords()
{
}

void World_CA_Generation::setup_biome_noises()
{
    std::vector<Biome_Type> biome_types = {
        Biome_Type::SANDY,
        Biome_Type::STONE,
        Biome_Type::ICY,
        Biome_Type::URANIUM};

    for (const auto &biome_type : biome_types)
    {
        Biome biome;

        switch (biome_type)
        {
        case Biome_Type::SANDY:
            biome = get_desert_biome();
            break;

        case Biome_Type::STONE:
            biome = get_stone_biome();
            break;

        case Biome_Type::ICY:
            biome = get_icy_biome();
            break;

        case Biome_Type::URANIUM:
            biome = get_uranium_biome();
            break;

        default:
            break;
        }

        FastNoiseLite noise;
        noise.SetSeed(seed);
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(4);
        noise.SetFrequency(biome.cave_size);

        biome_cave_noises[biome_type] = noise;
    }
}

void World_CA_Generation::set_seed(const int seed)
{
    this->seed = seed;

    recalculate_noises();
    setup_biome_noises();
}

void World_CA_Generation::fill_chunk(Chunk *chunk)
{
    chunk->set_chunk_data(default_chunk_data);
}

void World_CA_Generation::carve_cave_noise(Chunk *chunk)
{
    auto chunk_data = chunk->get_chunk_data();

    for (auto &cell : *chunk_data)
    {
        int world_x = chunk->coords.x * chunk->width + cell.coords.x;
        int world_y = chunk->coords.y * chunk->height + cell.coords.y;

        // float noise_value = cave_noise.GetNoise((float)world_x, (float)world_y);

        // if (noise_value < -0.2f)
        // {
        //     cell.set_particle(Particle());
        // }
    }
}

void World_CA_Generation::carve_cave_multinoise(Chunk *chunk)
{
    auto data = chunk->get_chunk_data();

    FastNoiseLite large_caves;
    large_caves.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    large_caves.SetFrequency(0.015f); // Big caves

    FastNoiseLite small_tunnels;
    small_tunnels.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    small_tunnels.SetFrequency(0.05f); // Small tunnels

    for (auto &cell : *data)
    {
        int world_x = chunk->coords.x * chunk->width + cell.coords.x;
        int world_y = chunk->coords.y * chunk->height + cell.coords.y;

        float large = large_caves.GetNoise((float)world_x, (float)world_y);
        float small = small_tunnels.GetNoise((float)world_x, (float)world_y);

        bool is_cave = (large < -0.3f) || (small < -0.5f);

        if (is_cave)
        {
            cell.set_particle(Particle());
        }
    }
}

void World_CA_Generation::carve_cave_iteration(Chunk *chunk)
{
}

Biome World_CA_Generation::get_biome(const int world_x, const int world_y)
{
    float noise = biome_noise.GetNoise((float)world_x, (float)world_y);

    if (noise < -0.5)
    {
        return get_desert_biome();
    }
    else if (-0.5 < noise && noise < 0.0)
    {
        return get_stone_biome();
    }
    else if (0.0 < noise && noise < 0.5)
    {
        return get_icy_biome();
    }
    else
    {
        return get_uranium_biome();
    }
}

void World_CA_Generation::generate_chunk_with_biome(Chunk *chunk)
{
    std::vector<WorldCell> new_data;
    new_data.reserve(chunk->width * chunk->height);

    for (int y = 0; y < chunk->height; y++)
    {
        for (int x = 0; x < chunk->width; x++)
        {
            int world_x = chunk->coords.x * chunk->width + x;
            int world_y = chunk->coords.y * chunk->height + y;

            Biome biome = get_biome(world_x, world_y);

            float cave_value = biome_cave_noises[biome.type].GetNoise((float)world_x, (float)world_y);

            if (cave_value < biome.cave_noise)
            {
                new_data.emplace_back(glm::ivec2(x, y));
            }
            else
            {
                switch (biome.particle_fill)
                {
                case Particle_Type::SAND:
                    new_data.emplace_back(glm::ivec2(x, y), create_sand());
                    break;
                case Particle_Type::STONE:
                    new_data.emplace_back(glm::ivec2(x, y), create_stone());
                    break;
                default:
                    new_data.emplace_back(glm::ivec2(x, y), create_stone());
                    break;
                }
            }
        }
    }

    chunk->set_chunk_data(new_data);
    chunk->set_state(Chunk_States::FINALL_FORM);
}

void World_CA_Generation::make_noise_data(Chunk *chunk)
{
    std::vector<WorldCell> new_chunk_data;
    new_chunk_data.reserve(chunk->width * chunk->height);

    for (int y{0}; y < chunk->height; y++)
    {
        for (int x{0}; x < chunk->width; x++)
        {
            if (Random_Machine::get_int_from_range(1, 100) < 46) // 46 je cave_noise density
            {
                new_chunk_data.emplace_back(glm::ivec2(x, y), create_stone());
            }
            else
            {
                new_chunk_data.emplace_back(glm::ivec2(x, y));
            }
        }
    }

    chunk->set_chunk_data(new_chunk_data);
}

void World_CA_Generation::iteration(Chunk *chunk)
{
    std::vector<WorldCell> new_chunk_data;
    new_chunk_data.reserve(chunk->width * chunk->height);

    auto neighbors_offsets = calculate_offsets_square(1);

    auto old_chunk_data = chunk->get_chunk_data();
    for (const auto &cell : *old_chunk_data)
    {
        int solid_neighbors = 0;

        for (const auto &offset : neighbors_offsets)
        {
            glm::ivec2 neighbor_coords = cell.coords + offset;

            if (!in_world_range(neighbor_coords.x, neighbor_coords.y, chunk->height, chunk->width))
            {
                solid_neighbors++;
                continue;
            }

            // auto neighbor = chunk->get_worldcell(neighbor_coords.x, neighbor_coords.y);
            auto neighbor2 = old_chunk_data->at(neighbor_coords.y * chunk->width + neighbor_coords.x);

            if (neighbor2.particle.type != Particle_Type::EMPTY)
            {
                solid_neighbors++;
            }
        }

        if (solid_neighbors > 4)
        {
            new_chunk_data.emplace_back(cell.coords, create_stone());
        }
        // else if (solid_neighbors <= 4)
        else
        {
            new_chunk_data.emplace_back(cell.coords);
        }
    }

    if (new_chunk_data == *old_chunk_data)
    {
        chunk->set_state(Chunk_States::FINALL_FORM);
    }

    chunk->set_chunk_data(new_chunk_data);
}
