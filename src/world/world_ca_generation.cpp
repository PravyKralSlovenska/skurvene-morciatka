#include "engine/world/world_ca_generation.hpp"

#include <iostream>
#include <algorithm>

#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/particle/particle.hpp"
#include "engine/world/world_biomes.hpp"
#include "others/utils.hpp"

World_CA_Generation::World_CA_Generation(const int chunk_width, const int chunk_height)
{
    // seed = Random_Machine::get_int_from_range(0, 9999);

    recalculate_noises();

    default_chunk_data.reserve(chunk_width * chunk_height);
    for (int y{0}; y < chunk_height; y++)
    {
        for (int x{0}; x < chunk_width; x++)
        {
            default_chunk_data.emplace_back(glm::ivec2(x, y), create_stone());
        }
    }
}

void World_CA_Generation::set_seed(const int seed)
{
    this->seed = seed;

    recalculate_noises();
    setup_biome_noises();
}

float World_CA_Generation::smoothstep(const float t, const float min, const float max)
{
    float value = std::clamp(t, min, max); // ak je hodnota mensia ako 0 vrati 0 ak je to naopak vrati 1 ak je niekde medzi vrati origo cislo
    return value * value * (3.0f - 2.0f * value);
}

float World_CA_Generation::linear_interpolation(const float a, const float b, const float t)
{
    return a + (b - a) * t;
}

void World_CA_Generation::recalculate_noises()
{
    biome_noise.SetSeed(seed);
    biome_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    biome_noise.SetFrequency(0.0002f);
    biome_noise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
    biome_noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    biome_noise.SetDomainWarpAmp(100.0f); // Strength of warping

    setup_biome_noises();
}

void World_CA_Generation::setup_biome_noises()
{
    Biome_Type biome_types[4] = {
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
        noise.SetFrequency(biome.cave_size);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(4);
        noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
        noise.SetDomainWarpAmp(30.0f);

        biome_cave_noises[biome_type] = noise;
    }
}

Particle World_CA_Generation::create_particle_by_type(const Particle_Type type)
{
    switch (type)
    {
    case Particle_Type::SAND:
        return create_sand();
    case Particle_Type::STONE:
        return create_stone();
    case Particle_Type::URANIUM:
        return create_uranium();
    case Particle_Type::WATER:
        return create_water();
    default:
        return create_stone();
    }
}

Particle_Type World_CA_Generation::blend_particle_types(const Particle_Type primary_type, const Particle_Type secondary_type, const float blend_factor, const glm::ivec2 &world_coords)
{
    // In transition zones, randomly choose based on blend factor
    // Use deterministic hash for consistent results
    int hash_value = hash_coords(world_coords.x, world_coords.y, seed);
    float random = (hash_value % 1000) / 1000.0f;

    // If blend_factor is 0.3, there's 70% chance of type1, 30% of type2
    return (random < blend_factor) ? primary_type : secondary_type;
}

inline glm::ivec2 World_CA_Generation::calculate_world_coords(const glm::ivec2 &local_coords, const glm::ivec2 &chunk_coords, const glm::ivec2 &chunk_dimensions)
{
    glm::ivec2 world_coords = (chunk_coords * chunk_dimensions) + local_coords;
    return world_coords;
}

inline float World_CA_Generation::get_noise_value(const FastNoiseLite &noise, const glm::ivec2 &world_coords)
{
    float noise_value = noise.GetNoise((float)world_coords.x, (float)world_coords.y);
    return noise_value;
}

Biome_Blend World_CA_Generation::get_biome_blend(const glm::ivec2 &world_coords)
{
    float noise = get_noise_value(biome_noise, world_coords);

    Biome_Blend result;

    // // Define biome ranges with overlap zones
    // const float DESERT_CENTER = -0.75f;
    // const float STONE_CENTER = -0.25f;
    // const float ICY_CENTER = 0.25f;
    // const float URANIUM_CENTER = 0.75f;

    // const float BLEND_RANGE = 0.3f; // How wide the transition zone is

    // // Find which two biomes we're between
    // if (noise < STONE_CENTER)
    // {
    //     // Between Desert and Stone
    //     result.primary = get_desert_biome();
    //     result.secondary = get_stone_biome();

    //     // Calculate blend factor (0 at desert center, 1 at stone center)
    //     float range = STONE_CENTER - DESERT_CENTER;
    //     result.blend_factor = (noise - DESERT_CENTER) / range;
    // }
    // else if (noise < ICY_CENTER)
    // {
    //     // Between Stone and Icy
    //     result.primary = get_stone_biome();
    //     result.secondary = get_icy_biome();

    //     float range = ICY_CENTER - STONE_CENTER;
    //     result.blend_factor = (noise - STONE_CENTER) / range;
    // }
    // else
    // {
    //     // Between Icy and Uranium
    //     result.primary = get_icy_biome();
    //     result.secondary = get_uranium_biome();

    //     float range = URANIUM_CENTER - ICY_CENTER;
    //     result.blend_factor = (noise - ICY_CENTER) / range;
    // }

    // // Apply smoothstep for more natural blending
    // result.blend_factor = smoothstep(result.blend_factor);

    return result;
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

Biome World_CA_Generation::get_biome(const glm::vec2 &world_coords)
{
    float noise = get_noise_value(biome_noise, world_coords);

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
    chunk->set_chunk_data(default_chunk_data);

    std::vector<WorldCell> new_data;
    new_data.reserve(chunk->width * chunk->height);

    for (const auto &cell : *chunk->get_chunk_data())
    {
        glm::ivec2 world_coords = calculate_world_coords(cell.coords, chunk->coords, chunk->get_chunk_dimensions());

        Biome biome = get_biome(world_coords);

        float cave_value = get_noise_value(biome_cave_noises[biome.type], world_coords);

        if (cave_value < biome.cave_noise)
        {
            new_data.emplace_back(cell.coords);
        }
        else
        {
            Particle particle = create_particle_by_type(biome.particle_fill);
            new_data.emplace_back(cell.coords, particle);
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
