#include "engine/world/world_ca_generation.hpp"

#include <iostream>
#include <algorithm>

#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/particle/particle.hpp"
#include "engine/world/world_biomes.hpp"
#include "others/utils.hpp"

namespace
{
    constexpr float BIOME_NOISE_FREQUENCY = 0.0010f;
    constexpr float BIOME_NOISE_GAIN = 1.50f;
    constexpr float BIOME_NOISE_BIAS = 0.05f;

    constexpr float SANDY_STONE_EDGE = -0.25f;
    constexpr float STONE_ICY_EDGE = 0.10f;
    constexpr float ICY_URANIUM_EDGE = 0.45f;
    constexpr float BIOME_BLEND_WIDTH = 0.12f;

    float remap_biome_noise(float raw_value)
    {
        return std::clamp(raw_value * BIOME_NOISE_GAIN + BIOME_NOISE_BIAS, -1.0f, 1.0f);
    }

    Biome biome_from_type(Biome_Type biome_type)
    {
        switch (biome_type)
        {
        case Biome_Type::SANDY:
            return get_desert_biome();
        case Biome_Type::STONE:
            return get_stone_biome();
        case Biome_Type::ICY:
            return get_icy_biome();
        case Biome_Type::URANIUM:
            return get_uranium_biome();
        default:
            return get_stone_biome();
        }
    }
}

World_CA_Generation::World_CA_Generation(const int chunk_width, const int chunk_height)
{
    // seed = Random_Machine::get_int_from_range(0, 9999);

    recalculate_noises();

    default_chunk_data.reserve(chunk_width * chunk_height);
    for (int y{0}; y < chunk_height; y++)
    {
        for (int x{0}; x < chunk_width; x++)
        {
            // World-generated particles are static (terrain)
            default_chunk_data.emplace_back(glm::ivec2(x, y), create_stone(true));
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
    biome_noise.SetFrequency(BIOME_NOISE_FREQUENCY);
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
    // World-generated particles are always static (terrain)
    switch (type)
    {
    case Particle_Type::SAND:
        return create_sand(true); // static = true
    case Particle_Type::STONE:
        return create_stone(true);
    case Particle_Type::URANIUM:
        return create_uranium(true);
    case Particle_Type::WATER:
        return create_water(true);
    case Particle_Type::ICE:
        return create_ice(true);
    case Particle_Type::WATER_VAPOR:
        return create_water_vapor(true);
    case Particle_Type::WOOD:
        return create_wood(true);
    case Particle_Type::FIRE:
        return create_fire(true);
    default:
        return create_stone(true);
    }
}

Particle_Type World_CA_Generation::blend_particle_types(const Particle_Type primary_type, const Particle_Type secondary_type, const float blend_factor, const glm::ivec2 &world_coords)
{
    // In transition zones, randomly choose based on blend factor
    // Use deterministic hash for consistent results
    int hash_value = hash_coords(world_coords.x, world_coords.y, seed);
    float random = (hash_value % 1000) / 1000.0f;

    // blend_factor = 0.0 => fully primary, 1.0 => fully secondary
    return (random < blend_factor) ? secondary_type : primary_type;
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
    const float noise = remap_biome_noise(get_noise_value(biome_noise, world_coords));
    const float half_blend = BIOME_BLEND_WIDTH * 0.5f;

    Biome_Blend result{
        get_stone_biome(),
        get_stone_biome(),
        0.0f};

    auto set_single = [&](Biome_Type biome_type)
    {
        const Biome biome = biome_from_type(biome_type);
        result.primary = biome;
        result.secondary = biome;
        result.blend_factor = 0.0f;
    };

    auto set_transition = [&](Biome_Type primary_type, Biome_Type secondary_type, float edge)
    {
        const float start = edge - half_blend;
        const float end = edge + half_blend;
        const float t = smoothstep((noise - start) / (end - start));
        result.primary = biome_from_type(primary_type);
        result.secondary = biome_from_type(secondary_type);
        result.blend_factor = t;
    };

    if (noise < SANDY_STONE_EDGE - half_blend)
    {
        set_single(Biome_Type::SANDY);
    }
    else if (noise <= SANDY_STONE_EDGE + half_blend)
    {
        set_transition(Biome_Type::SANDY, Biome_Type::STONE, SANDY_STONE_EDGE);
    }
    else if (noise < STONE_ICY_EDGE - half_blend)
    {
        set_single(Biome_Type::STONE);
    }
    else if (noise <= STONE_ICY_EDGE + half_blend)
    {
        set_transition(Biome_Type::STONE, Biome_Type::ICY, STONE_ICY_EDGE);
    }
    else if (noise < ICY_URANIUM_EDGE - half_blend)
    {
        set_single(Biome_Type::ICY);
    }
    else if (noise <= ICY_URANIUM_EDGE + half_blend)
    {
        set_transition(Biome_Type::ICY, Biome_Type::URANIUM, ICY_URANIUM_EDGE);
    }
    else
    {
        set_single(Biome_Type::URANIUM);
    }

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
    const glm::ivec2 cell_coords(
        static_cast<int>(std::floor(world_coords.x)),
        static_cast<int>(std::floor(world_coords.y)));

    const Biome_Blend blend = get_biome_blend(cell_coords);

    // Return dominant biome in transition regions.
    if (blend.blend_factor >= 0.5f)
        return blend.secondary;
    return blend.primary;
}

bool World_CA_Generation::is_cell_solid(int world_cell_x, int world_cell_y)
{
    const glm::ivec2 world_coords(world_cell_x, world_cell_y);
    const Biome_Blend biome_blend = get_biome_blend(world_coords);

    const float primary_cave = get_noise_value(biome_cave_noises[biome_blend.primary.type], world_coords);
    const float secondary_cave = get_noise_value(biome_cave_noises[biome_blend.secondary.type], world_coords);

    const float blended_cave = linear_interpolation(primary_cave, secondary_cave, biome_blend.blend_factor);
    const float blended_threshold = linear_interpolation(biome_blend.primary.cave_noise,
                                                         biome_blend.secondary.cave_noise,
                                                         biome_blend.blend_factor);
    return blended_cave >= blended_threshold;
}

void World_CA_Generation::generate_chunk_with_biome(Chunk *chunk)
{
    chunk->set_chunk_data(default_chunk_data);

    std::vector<WorldCell> new_data;
    new_data.reserve(chunk->width * chunk->height);

    for (const auto &cell : *chunk->get_chunk_data())
    {
        const glm::ivec2 world_coords = calculate_world_coords(cell.coords, chunk->coords, chunk->get_chunk_dimensions());
        const Biome_Blend biome_blend = get_biome_blend(world_coords);

        const float primary_cave = get_noise_value(biome_cave_noises[biome_blend.primary.type], world_coords);
        const float secondary_cave = get_noise_value(biome_cave_noises[biome_blend.secondary.type], world_coords);
        const float blended_cave = linear_interpolation(primary_cave, secondary_cave, biome_blend.blend_factor);

        const float blended_threshold = linear_interpolation(biome_blend.primary.cave_noise,
                                                             biome_blend.secondary.cave_noise,
                                                             biome_blend.blend_factor);

        if (blended_cave < blended_threshold)
        {
            new_data.emplace_back(cell.coords);
        }
        else
        {
            const Particle_Type particle_type = blend_particle_types(biome_blend.primary.particle_fill,
                                                                     biome_blend.secondary.particle_fill,
                                                                     biome_blend.blend_factor,
                                                                     world_coords);
            const Particle particle = create_particle_by_type(particle_type);
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
