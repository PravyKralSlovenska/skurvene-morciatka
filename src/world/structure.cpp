#include "engine/world/structure.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/world/world_ca_generation.hpp"
#include "engine/world/world_biomes.hpp"
#include "others/GLOBALS.hpp"
#include "others/utils.hpp"

#include "stb/stb_image.h"

#include <filesystem>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <memory>
#include <cstdint>
#include <limits>
#include <sstream>
#include <unordered_map>

static int pixel_to_cell(int px)
{
    return static_cast<int>(std::floor(static_cast<float>(px) / Globals::PARTICLE_SIZE));
}

static bool is_store_proof_logging_enabled()
{
    static const bool enabled = []
    {
        const char *value = std::getenv("MORCIATKO_STORE_PROOF");
        return value != nullptr && value[0] != '\0' && !(value[0] == '0' && value[1] == '\0');
    }();

    return enabled;
}

static std::uint64_t splitmix64(std::uint64_t value)
{
    value += 0x9E3779B97F4A7C15ull;
    value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ull;
    value = (value ^ (value >> 27)) * 0x94D049BB133111EBull;
    return value ^ (value >> 31);
}

static int floor_div_int(int value, int divisor)
{
    if (divisor <= 0)
        return 0;

    int quotient = value / divisor;
    const int remainder = value % divisor;
    if (remainder != 0 && value < 0)
        quotient -= 1;
    return quotient;
}

// ============================================
// Structure
// ============================================

Structure::Structure(const std::string &name, int width, int height)
    : name(name), width(width), height(height), cells(width * height, Particle()) {}

void Structure::set_cell(int x, int y, const Particle &particle)
{
    if (!in_bounds(x, y))
        return;
    cells[y * width + x] = particle;
}

void Structure::set_cell(int x, int y, Particle_Type type, bool is_static)
{
    if (!in_bounds(x, y))
        return;

    Particle p;
    switch (type)
    {
    case Particle_Type::SAND:
        p = create_sand(is_static);
        break;
    case Particle_Type::WATER:
        p = create_water(is_static);
        break;
    case Particle_Type::ICE:
        p = create_ice(is_static);
        break;
    case Particle_Type::WATER_VAPOR:
        p = create_water_vapor(is_static);
        break;
    case Particle_Type::SMOKE:
        p = create_smoke(is_static);
        break;
    case Particle_Type::WOOD:
        p = create_wood(is_static);
        break;
    case Particle_Type::FIRE:
        p = create_fire(is_static);
        break;
    case Particle_Type::URANIUM:
        p = create_uranium(is_static);
        break;
    case Particle_Type::STONE:
        p = create_stone(is_static);
        break;
    default:
        p = Particle();
        break;
    }
    cells[y * width + x] = p;
}

void Structure::fill_rect(int x, int y, int w, int h, Particle_Type type, bool is_static)
{
    for (int ry = y; ry < y + h; ry++)
    {
        for (int rx = x; rx < x + w; rx++)
        {
            set_cell(rx, ry, type, is_static);
        }
    }
}

const Particle &Structure::get_cell(int x, int y) const
{
    static Particle empty_particle;
    if (!in_bounds(x, y))
        return empty_particle;
    return cells[y * width + x];
}

bool Structure::in_bounds(int x, int y) const
{
    return x >= 0 && x < width && y >= 0 && y < height;
}

int Structure::count_solid_cells() const
{
    int count = 0;
    for (const auto &cell : cells)
    {
        if (cell.type != Particle_Type::EMPTY)
            count++;
    }
    return count;
}

// ============================================
// StructureFactory
// ============================================

namespace StructureFactory
{
    Structure create_platform(int length, Particle_Type material)
    {
        int height = 3;
        Structure s("platform", length, height);
        s.fill_rect(0, 0, length, height, material, true);
        return s;
    }
}

// ============================================
// StructureSpawner
// ============================================

StructureSpawner::StructureSpawner() : rng(0) {}

void StructureSpawner::set_world(World *world)
{
    this->world = world;
}

void StructureSpawner::set_seed(int seed)
{
    this->seed = seed;
    rng.seed(seed);
}

void StructureSpawner::set_structure_spawn_count(const std::string &name, int count)
{
    structure_spawn_counts[name] = std::max(1, count);
}

void StructureSpawner::set_devushki_spawn_radius_particles(int radius_particles)
{
    devushki_spawn_radius_particles = std::max(100, radius_particles);
}

int StructureSpawner::get_devushki_spawn_radius_particles() const
{
    return devushki_spawn_radius_particles;
}

StructureSpawner::PlacementProfile StructureSpawner::sanitize_placement_profile(const PlacementProfile &profile) const
{
    PlacementProfile sanitized = profile;
    sanitized.min_supported_columns = std::max(1, sanitized.min_supported_columns);
    sanitized.min_support_ratio = std::clamp(sanitized.min_support_ratio, 0.0f, 1.0f);
    sanitized.support_scan_max_depth_cells = std::max(1, sanitized.support_scan_max_depth_cells);
    sanitized.max_ground_variation_cells = std::max(0, sanitized.max_ground_variation_cells);
    return sanitized;
}

void StructureSpawner::set_default_placement_profile(const PlacementProfile &profile)
{
    default_placement_profile = sanitize_placement_profile(profile);
}

const StructureSpawner::PlacementProfile &StructureSpawner::get_default_placement_profile() const
{
    return default_placement_profile;
}

void StructureSpawner::set_structure_placement_profile(const std::string &name, const PlacementProfile &profile)
{
    structure_placement_profiles[name] = sanitize_placement_profile(profile);
}

bool StructureSpawner::get_structure_placement_profile(const std::string &name, PlacementProfile &out_profile) const
{
    auto it = structure_placement_profiles.find(name);
    if (it == structure_placement_profiles.end())
        return false;

    out_profile = it->second;
    return true;
}

void StructureSpawner::clear_structure_placement_profile(const std::string &name)
{
    structure_placement_profiles.erase(name);
}

StructureSpawner::PlacementProfile StructureSpawner::resolve_structure_placement_profile(const std::string &name) const
{
    auto it = structure_placement_profiles.find(name);
    if (it != structure_placement_profiles.end())
        return it->second;

    return default_placement_profile;
}

void StructureSpawner::set_store_spawn_config(const StoreSpawnConfig &config)
{
    StoreSpawnConfig sanitized = config;
    sanitized.chunks_per_spawn = std::max(1, sanitized.chunks_per_spawn);
    sanitized.min_generated_chunks_before_first_store = std::max(0, sanitized.min_generated_chunks_before_first_store);
    sanitized.min_spawn_radius_particles = std::max(0, sanitized.min_spawn_radius_particles);
    sanitized.max_spawn_radius_particles = std::max(sanitized.min_spawn_radius_particles, sanitized.max_spawn_radius_particles);
    sanitized.min_distance_between_stores_particles = std::max(0, sanitized.min_distance_between_stores_particles);
    sanitized.min_distance_from_columns_particles = std::max(0, sanitized.min_distance_from_columns_particles);
    sanitized.min_distance_from_origin_particles = std::max(0, sanitized.min_distance_from_origin_particles);
    sanitized.spacing_chunks = std::max(0, sanitized.spacing_chunks);
    sanitized.separation_chunks = std::max(0, sanitized.separation_chunks);
    sanitized.active_region_radius_chunks = std::max(1, sanitized.active_region_radius_chunks);
    sanitized.max_regions_processed_per_call = std::max(1, sanitized.max_regions_processed_per_call);
    sanitized.max_attempts_per_region = std::max(1, sanitized.max_attempts_per_region);
    sanitized.retry_generated_chunks = std::max(0, sanitized.retry_generated_chunks);

    store_spawn_config = sanitized;

    store_regions.clear();
    store_region_queue.clear();
    store_region_queued.clear();

    if (!store_spawn_config.enabled)
    {
        predetermined_entries.erase(
            std::remove_if(predetermined_entries.begin(), predetermined_entries.end(),
                           [&](const PredeterminedEntry &entry)
                           {
                               return is_store_name(entry.structure_name) && !entry.placed;
                           }),
            predetermined_entries.end());
    }
}

const StructureSpawner::StoreSpawnConfig &StructureSpawner::get_store_spawn_config() const
{
    return store_spawn_config;
}

void StructureSpawner::set_store_spawning_enabled(bool enabled)
{
    StoreSpawnConfig updated = store_spawn_config;
    updated.enabled = enabled;
    set_store_spawn_config(updated);
}

bool StructureSpawner::is_store_spawning_enabled() const
{
    return store_spawn_config.enabled;
}

bool StructureSpawner::is_store_name(const std::string &name) const
{
    return name == "store" || name == "devushki_store";
}

bool StructureSpawner::is_column_name(const std::string &name) const
{
    return name == "devushki_column";
}

bool StructureSpawner::is_store_target_far_enough(const glm::ivec2 &target_px) const
{
    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    const long long min_origin_px = static_cast<long long>(store_spawn_config.min_distance_from_origin_particles) * ps;
    const long long dist_origin_sq =
        static_cast<long long>(target_px.x) * static_cast<long long>(target_px.x) +
        static_cast<long long>(target_px.y) * static_cast<long long>(target_px.y);

    if (dist_origin_sq < min_origin_px * min_origin_px)
        return false;

    const long long min_store_spacing_px = static_cast<long long>(store_spawn_config.min_distance_between_stores_particles) * ps;
    const long long min_store_spacing_sq = min_store_spacing_px * min_store_spacing_px;
    const long long min_column_spacing_px = static_cast<long long>(store_spawn_config.min_distance_from_columns_particles) * ps;
    const long long min_column_spacing_sq = min_column_spacing_px * min_column_spacing_px;

    for (const auto &entry : predetermined_entries)
    {
        if (!is_store_name(entry.structure_name))
            continue;

        const long long dx = static_cast<long long>(target_px.x) - static_cast<long long>(entry.target_pos.x);
        const long long dy = static_cast<long long>(target_px.y) - static_cast<long long>(entry.target_pos.y);
        if (dx * dx + dy * dy < min_store_spacing_sq)
            return false;
    }

    for (const auto &entry : predetermined_entries)
    {
        if (!is_column_name(entry.structure_name))
            continue;

        const long long dx = static_cast<long long>(target_px.x) - static_cast<long long>(entry.target_pos.x);
        const long long dy = static_cast<long long>(target_px.y) - static_cast<long long>(entry.target_pos.y);
        if (dx * dx + dy * dy < min_column_spacing_sq)
            return false;
    }

    for (const auto &placed : placed_structures)
    {
        if (!is_store_name(placed.name))
            continue;

        const long long dx = static_cast<long long>(target_px.x) - static_cast<long long>(placed.position.x);
        const long long dy = static_cast<long long>(target_px.y) - static_cast<long long>(placed.position.y);
        if (dx * dx + dy * dy < min_store_spacing_sq)
            return false;
    }

    for (const auto &placed : placed_structures)
    {
        if (!is_column_name(placed.name))
            continue;

        const long long dx = static_cast<long long>(target_px.x) - static_cast<long long>(placed.position.x);
        const long long dy = static_cast<long long>(target_px.y) - static_cast<long long>(placed.position.y);
        if (dx * dx + dy * dy < min_column_spacing_sq)
            return false;
    }

    return true;
}

bool StructureSpawner::try_generate_store_target_for_sequence(int sequence_index, glm::ivec2 &out_target_px) const
{
    if (!world)
        return false;

    auto *chunks = world->get_chunks();
    if (!chunks || chunks->empty())
        return false;

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
    const int chunk_pixel_w = chunk_dims.x * ps;
    const int chunk_pixel_h = chunk_dims.y * ps;

    int min_chunk_x = std::numeric_limits<int>::max();
    int max_chunk_x = std::numeric_limits<int>::min();
    int min_chunk_y = std::numeric_limits<int>::max();
    int max_chunk_y = std::numeric_limits<int>::min();

    for (const auto &pair : *chunks)
    {
        const glm::ivec2 &coords = pair.first;
        min_chunk_x = std::min(min_chunk_x, coords.x);
        max_chunk_x = std::max(max_chunk_x, coords.x);
        min_chunk_y = std::min(min_chunk_y, coords.y);
        max_chunk_y = std::max(max_chunk_y, coords.y);
    }

    const float center_chunk_x = 0.5f * static_cast<float>(min_chunk_x + max_chunk_x);
    const float center_chunk_y = 0.5f * static_cast<float>(min_chunk_y + max_chunk_y);
    const glm::ivec2 explored_center_px(
        static_cast<int>((center_chunk_x + 0.5f) * static_cast<float>(chunk_pixel_w)),
        static_cast<int>((center_chunk_y + 0.5f) * static_cast<float>(chunk_pixel_h)));

    const int span_chunks_x = max_chunk_x - min_chunk_x + 1;
    const int span_chunks_y = max_chunk_y - min_chunk_y + 1;
    const int explored_radius_chunks = std::max(span_chunks_x, span_chunks_y) / 2;
    const int explored_radius_px = explored_radius_chunks * std::max(chunk_pixel_w, chunk_pixel_h);

    const int configured_min_radius_px = std::max(0, store_spawn_config.min_spawn_radius_particles * ps);
    const int configured_max_radius_px = std::max(configured_min_radius_px, store_spawn_config.max_spawn_radius_particles * ps);
    const long long min_origin_px = static_cast<long long>(store_spawn_config.min_distance_from_origin_particles) * ps;
    const long long min_origin_sq = min_origin_px * min_origin_px;
    const long long min_store_spacing_px = static_cast<long long>(store_spawn_config.min_distance_between_stores_particles) * ps;
    const long long min_store_spacing_sq = min_store_spacing_px * min_store_spacing_px;

    const int explored_shell_padding_px = std::max(chunk_pixel_w, chunk_pixel_h) * 2;
    const int min_radius_px = std::max(configured_min_radius_px, explored_radius_px + explored_shell_padding_px);
    const int max_radius_px = std::max(min_radius_px + ps, configured_max_radius_px + explored_radius_px / 2);

    const std::uint32_t sequence_hash = static_cast<std::uint32_t>(sequence_index + 1) * 2654435761u;
    std::mt19937 target_rng(static_cast<std::uint32_t>(seed) ^ sequence_hash);
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * static_cast<float>(M_PI));
    std::uniform_int_distribution<int> radius_dist(min_radius_px, max_radius_px);

    std::unordered_map<glm::ivec2, std::vector<glm::ivec2>, Chunk_Coords_to_Hash> store_buckets;
    const int bucket_size_px = std::max(ps, static_cast<int>(min_store_spacing_px));

    auto floor_div = [](int value, int divisor)
    {
        if (divisor <= 0)
            return 0;

        int quotient = value / divisor;
        const int remainder = value % divisor;
        if (remainder != 0 && value < 0)
            quotient -= 1;
        return quotient;
    };

    auto bucket_coords_for = [&](const glm::ivec2 &position)
    {
        return glm::ivec2(
            floor_div(position.x, bucket_size_px),
            floor_div(position.y, bucket_size_px));
    };

    if (min_store_spacing_sq > 0)
    {
        store_buckets.reserve(predetermined_entries.size() + placed_structures.size());

        for (const auto &entry : predetermined_entries)
        {
            if (!is_store_name(entry.structure_name))
                continue;

            store_buckets[bucket_coords_for(entry.target_pos)].push_back(entry.target_pos);
        }

        for (const auto &placed : placed_structures)
        {
            if (!is_store_name(placed.name))
                continue;

            store_buckets[bucket_coords_for(placed.position)].push_back(placed.position);
        }
    }

    auto is_store_target_far_enough_fast = [&](const glm::ivec2 &candidate)
    {
        const long long dist_origin_sq =
            static_cast<long long>(candidate.x) * static_cast<long long>(candidate.x) +
            static_cast<long long>(candidate.y) * static_cast<long long>(candidate.y);
        if (dist_origin_sq < min_origin_sq)
            return false;

        if (min_store_spacing_sq <= 0)
            return true;

        const glm::ivec2 center_bucket = bucket_coords_for(candidate);
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                const glm::ivec2 neighbor_bucket(center_bucket.x + dx, center_bucket.y + dy);
                const auto it = store_buckets.find(neighbor_bucket);
                if (it == store_buckets.end())
                    continue;

                for (const auto &position : it->second)
                {
                    const long long diff_x = static_cast<long long>(candidate.x) - static_cast<long long>(position.x);
                    const long long diff_y = static_cast<long long>(candidate.y) - static_cast<long long>(position.y);
                    if (diff_x * diff_x + diff_y * diff_y < min_store_spacing_sq)
                        return false;
                }
            }
        }

        return true;
    };

    constexpr int STORE_TARGET_ATTEMPTS_PER_SEQUENCE = 24;
    for (int attempt = 0; attempt < STORE_TARGET_ATTEMPTS_PER_SEQUENCE; ++attempt)
    {
        const float angle = angle_dist(target_rng);
        const int radius = radius_dist(target_rng);

        int candidate_x = explored_center_px.x + static_cast<int>(std::cos(angle) * static_cast<float>(radius));
        int candidate_y = explored_center_px.y + static_cast<int>(std::sin(angle) * static_cast<float>(radius));
        candidate_x = static_cast<int>(std::floor(static_cast<float>(candidate_x) / static_cast<float>(ps))) * ps;
        candidate_y = static_cast<int>(std::floor(static_cast<float>(candidate_y) / static_cast<float>(ps))) * ps;

        const glm::ivec2 candidate(candidate_x, candidate_y);
        if (!is_store_target_far_enough_fast(candidate))
            continue;

        out_target_px = candidate;
        return true;
    }

    return false;
}

void StructureSpawner::add_blueprint(const std::string &name, const Structure &structure)
{
    blueprints[name] = structure;
}

void StructureSpawner::add_blueprint(Structure &&structure)
{
    std::string name = structure.get_name();
    blueprints[name] = std::move(structure);
}

Structure *StructureSpawner::get_blueprint(const std::string &name)
{
    auto it = blueprints.find(name);
    if (it != blueprints.end())
        return &it->second;
    return nullptr;
}

const std::map<std::string, Structure> &StructureSpawner::get_blueprints() const
{
    return blueprints;
}

bool StructureSpawner::place_structure(const Structure &structure, const glm::ivec2 &world_pos)
{
    if (!world)
        return false;

    for (int y = 0; y < structure.get_height(); y++)
    {
        for (int x = 0; x < structure.get_width(); x++)
        {
            const Particle &cell = structure.get_cell(x, y);
            if (cell.type == Particle_Type::EMPTY)
                continue;

            glm::ivec2 pixel_pos = world_pos + glm::ivec2(
                                                   static_cast<int>(x * Globals::PARTICLE_SIZE),
                                                   static_cast<int>(y * Globals::PARTICLE_SIZE));
            world->place_custom_particle(pixel_pos, cell);
        }
    }

    fill_structure_base(structure, world_pos);
    return true;
}

bool StructureSpawner::place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos)
{
    glm::ivec2 offset(
        static_cast<int>((structure.get_width() / 2) * Globals::PARTICLE_SIZE),
        static_cast<int>((structure.get_height() / 2) * Globals::PARTICLE_SIZE));
    return place_structure(structure, center_pos - offset);
}

namespace
{
    static constexpr int RANDOM_TARGET_MIN = -15000;
    static constexpr int RANDOM_TARGET_MAX = 15000;
    static constexpr int MAX_VERTICAL_LIFT_CELLS = 120;
    static constexpr int MAX_RAW_RETRY_ATTEMPTS = 6;
    static constexpr int RAW_RETRY_DISTANCE_CELLS = 250;
    static constexpr int GROUND_FILL_MAX_DEPTH_CELLS = 512;
    static constexpr float BASE_BOWL_EDGE_DEPTH_RATIO = 0.35f;
    static constexpr int BASE_BOWL_SMOOTH_DELTA_CELLS = 2;
    static constexpr int BASE_FEATHER_SIDE_SPREAD_CELLS = 3;
    static constexpr int BASE_FEATHER_MAX_DEPTH_CELLS = 24;
    static constexpr int STRUCTURE_CLEARANCE_CELLS = 1;
    static constexpr int STORE_TARGET_MAX_SEQUENCE_ADVANCE = 32;
    static constexpr int ENTRY_PLACE_RADIUS_CHUNKS = 10;
    static constexpr int NON_COLUMN_DEFAULT_SPAWN_OPPORTUNITIES = 10;

    struct ReservedPlacement
    {
        glm::ivec2 position;
        const Structure *blueprint = nullptr;
    };

    using ProbeChunkCache = std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash>;

    int align_to_particle_grid(int value, int particle_size)
    {
        if (particle_size <= 0)
            return value;

        return static_cast<int>(std::floor(static_cast<float>(value) / static_cast<float>(particle_size))) * particle_size;
    }

    float deterministic_noise_01(int x, int y, int salt)
    {
        std::uint32_t h = 2166136261u;
        h = (h ^ static_cast<std::uint32_t>(x * 73856093)) * 16777619u;
        h = (h ^ static_cast<std::uint32_t>(y * 19349663)) * 16777619u;
        h = (h ^ static_cast<std::uint32_t>(salt * 83492791)) * 16777619u;
        return static_cast<float>(h & 0xFFFFu) / 65535.0f;
    }

    glm::ivec2 get_chunk_pos_from_world_px(int world_x, int world_y, int chunk_pixel_w, int chunk_pixel_h)
    {
        return glm::ivec2(
            static_cast<int>(std::floor(static_cast<float>(world_x) / static_cast<float>(chunk_pixel_w))),
            static_cast<int>(std::floor(static_cast<float>(world_y) / static_cast<float>(chunk_pixel_h))));
    }

    Particle_Type get_support_particle_type_for_world_px(World *world, int world_x, int world_y)
    {
        if (!world)
            return Particle_Type::STONE;

        World_CA_Generation *world_gen = world->get_world_gen();
        if (!world_gen)
            return Particle_Type::STONE;

        const glm::vec2 world_cell_coords(
            static_cast<float>(pixel_to_cell(world_x)),
            static_cast<float>(pixel_to_cell(world_y)));

        const Biome biome = world_gen->get_biome(world_cell_coords);
        return biome.particle_fill;
    }

    bool get_chunk_and_local_cell(World *world, int world_x, int world_y,
                                  Chunk *&out_chunk, glm::ivec2 &out_local_cell)
    {
        if (!world)
            return false;

        const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        const int chunk_pixel_w = chunk_dims.x * ps;
        const int chunk_pixel_h = chunk_dims.y * ps;

        glm::ivec2 chunk_pos = get_chunk_pos_from_world_px(world_x, world_y, chunk_pixel_w, chunk_pixel_h);
        Chunk *chunk = world->get_chunk(chunk_pos);
        if (!chunk)
            return false;

        int offset_x = world_x - chunk_pos.x * chunk_pixel_w;
        int offset_y = world_y - chunk_pos.y * chunk_pixel_h;
        int cell_x = offset_x / ps;
        int cell_y = offset_y / ps;

        if (cell_x < 0)
            cell_x += chunk_dims.x;
        if (cell_y < 0)
            cell_y += chunk_dims.y;

        if (cell_x < 0 || cell_x >= chunk_dims.x || cell_y < 0 || cell_y >= chunk_dims.y)
            return false;

        out_chunk = chunk;
        out_local_cell = glm::ivec2(cell_x, cell_y);
        return true;
    }

    bool ensure_chunk_generated(World *world, const glm::ivec2 &chunk_coords)
    {
        if (!world)
            return false;

        if (world->get_chunk(chunk_coords))
            return true;

        auto *chunks = world->get_chunks();
        World_CA_Generation *world_gen = world->get_world_gen();
        if (!chunks || !world_gen)
            return false;

        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        auto chunk = std::make_unique<Chunk>(chunk_coords, chunk_dims.x, chunk_dims.y);
        world_gen->generate_chunk_with_biome(chunk.get());
        chunks->emplace(chunk_coords, std::move(chunk));
        return true;
    }

    Chunk *get_or_create_probe_chunk(World *world, ProbeChunkCache &probe_chunks, const glm::ivec2 &chunk_coords)
    {
        if (!world)
            return nullptr;

        if (Chunk *existing = world->get_chunk(chunk_coords))
            return existing;

        auto it = probe_chunks.find(chunk_coords);
        if (it != probe_chunks.end())
            return it->second.get();

        World_CA_Generation *world_gen = world->get_world_gen();
        if (!world_gen)
            return nullptr;

        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        auto chunk = std::make_unique<Chunk>(chunk_coords, chunk_dims.x, chunk_dims.y);
        world_gen->generate_chunk_with_biome(chunk.get());

        Chunk *chunk_ptr = chunk.get();
        probe_chunks.emplace(chunk_coords, std::move(chunk));
        return chunk_ptr;
    }

    bool get_probe_chunk_and_local_cell(World *world, ProbeChunkCache &probe_chunks,
                                        int world_x, int world_y,
                                        Chunk *&out_chunk, glm::ivec2 &out_local_cell)
    {
        if (!world)
            return false;

        const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        const int chunk_pixel_w = chunk_dims.x * ps;
        const int chunk_pixel_h = chunk_dims.y * ps;

        const glm::ivec2 chunk_pos = get_chunk_pos_from_world_px(world_x, world_y, chunk_pixel_w, chunk_pixel_h);
        Chunk *chunk = get_or_create_probe_chunk(world, probe_chunks, chunk_pos);
        if (!chunk)
            return false;

        int offset_x = world_x - chunk_pos.x * chunk_pixel_w;
        int offset_y = world_y - chunk_pos.y * chunk_pixel_h;
        int cell_x = offset_x / ps;
        int cell_y = offset_y / ps;

        if (cell_x < 0)
            cell_x += chunk_dims.x;
        if (cell_y < 0)
            cell_y += chunk_dims.y;

        if (cell_x < 0 || cell_x >= chunk_dims.x || cell_y < 0 || cell_y >= chunk_dims.y)
            return false;

        out_chunk = chunk;
        out_local_cell = glm::ivec2(cell_x, cell_y);
        return true;
    }

    bool overlaps_reserved(const Structure &candidate_structure,
                           const glm::ivec2 &candidate_pos,
                           const std::vector<ReservedPlacement> &reserved,
                           int ps)
    {
        const int clearance_px = STRUCTURE_CLEARANCE_CELLS * ps;
        const int cand_left = candidate_pos.x - clearance_px;
        const int cand_top = candidate_pos.y - clearance_px;
        const int cand_right = candidate_pos.x + candidate_structure.get_width() * ps + clearance_px;
        const int cand_bottom = candidate_pos.y + candidate_structure.get_height() * ps + clearance_px;
        const int cand_left_nominal = candidate_pos.x;
        const int cand_top_nominal = candidate_pos.y;
        const int cand_right_nominal = candidate_pos.x + candidate_structure.get_width() * ps;
        const int cand_bottom_nominal = candidate_pos.y + candidate_structure.get_height() * ps;

        for (const auto &placed : reserved)
        {
            if (!placed.blueprint)
                continue;

            const int placed_left = placed.position.x;
            const int placed_top = placed.position.y;
            const int placed_right = placed.position.x + placed.blueprint->get_width() * ps;
            const int placed_bottom = placed.position.y + placed.blueprint->get_height() * ps;

            const bool separated =
                cand_right <= placed_left ||
                cand_left >= placed_right ||
                cand_bottom <= placed_top ||
                cand_top >= placed_bottom;

            if (!separated)
                return true;

            const bool x_overlap =
                cand_right_nominal > placed_left &&
                cand_left_nominal < placed_right;

            if (x_overlap)
            {
                const bool sits_on_top =
                    cand_bottom_nominal >= placed_top - ps &&
                    cand_bottom_nominal <= placed_top + ps;

                const bool sits_below =
                    placed_bottom >= cand_top_nominal - ps &&
                    placed_bottom <= cand_top_nominal + ps;

                if (sits_on_top || sits_below)
                    return true;
            }
        }

        return false;
    }

    bool is_valid_placement_probe(World *world,
                                  const Structure &structure,
                                  const glm::ivec2 &world_pos,
                                  const std::vector<ReservedPlacement> &reserved,
                                  ProbeChunkCache &probe_chunks)
    {
        if (!world)
            return false;

        const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
        if (overlaps_reserved(structure, world_pos, reserved, ps))
            return false;

        for (int sy = 0; sy < structure.get_height(); ++sy)
        {
            for (int sx = 0; sx < structure.get_width(); ++sx)
            {
                const Particle &cell = structure.get_cell(sx, sy);
                if (cell.type == Particle_Type::EMPTY)
                    continue;

                const int world_x = world_pos.x + sx * ps;
                const int world_y = world_pos.y + sy * ps;

                Chunk *chunk = nullptr;
                glm::ivec2 local_cell;
                if (!get_probe_chunk_and_local_cell(world, probe_chunks, world_x, world_y, chunk, local_cell))
                    return false;

                if (!chunk->is_empty(local_cell.x, local_cell.y))
                    return false;
            }
        }

        const int ground_y = world_pos.y + structure.get_height() * ps;
        int supported_columns = 0;
        for (int sx = 0; sx < structure.get_width(); ++sx)
        {
            const int world_x = world_pos.x + sx * ps;
            Chunk *chunk = nullptr;
            glm::ivec2 local_cell;
            if (!get_probe_chunk_and_local_cell(world, probe_chunks, world_x, ground_y, chunk, local_cell))
                return false;

            if (!chunk->is_empty(local_cell.x, local_cell.y))
                ++supported_columns;
        }

        return supported_columns > 0;
    }

    bool resolve_valid_target_for_entry(World *world,
                                        const Structure &blueprint,
                                        const glm::ivec2 &original_target,
                                        std::size_t entry_index,
                                        int retry_phase,
                                        const std::vector<ReservedPlacement> &reserved,
                                        glm::ivec2 &resolved_out)
    {
        if (!world)
            return false;

        ProbeChunkCache probe_chunks;
        const int ps = static_cast<int>(Globals::PARTICLE_SIZE);

        glm::ivec2 raw_pos = original_target;
        std::mt19937 retry_rng(static_cast<std::uint32_t>(entry_index * 73856093u) ^ static_cast<std::uint32_t>(retry_phase * 19349663u));
        std::uniform_int_distribution<int> retry_offset(-RAW_RETRY_DISTANCE_CELLS * ps,
                                                        RAW_RETRY_DISTANCE_CELLS * ps);

        for (int attempt = 0; attempt < MAX_RAW_RETRY_ATTEMPTS; ++attempt)
        {
            if (attempt > 0)
            {
                raw_pos.x += retry_offset(retry_rng);
                raw_pos.y += retry_offset(retry_rng);
            }

            raw_pos.x = align_to_particle_grid(raw_pos.x, ps);
            raw_pos.y = align_to_particle_grid(raw_pos.y, ps);

            for (int lift = 0; lift <= MAX_VERTICAL_LIFT_CELLS; ++lift)
            {
                glm::ivec2 candidate_pos(raw_pos.x, raw_pos.y - lift * ps);
                if (!is_valid_placement_probe(world, blueprint, candidate_pos, reserved, probe_chunks))
                    continue;

                resolved_out = candidate_pos;
                return true;
            }
        }

        return false;
    }

    bool ensure_footprint_chunks_generated(World *world, const Structure &structure, const glm::ivec2 &world_pos)
    {
        if (!world)
            return false;

        const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        const int chunk_pixel_w = chunk_dims.x * ps;
        const int chunk_pixel_h = chunk_dims.y * ps;

        const int struct_pw = structure.get_width() * ps;
        const int struct_ph = structure.get_height() * ps;

        glm::ivec2 first_chunk = get_chunk_pos_from_world_px(world_pos.x, world_pos.y, chunk_pixel_w, chunk_pixel_h);
        glm::ivec2 last_chunk = get_chunk_pos_from_world_px(world_pos.x + struct_pw - 1,
                                                            world_pos.y + struct_ph - 1,
                                                            chunk_pixel_w, chunk_pixel_h);

        for (int cy = first_chunk.y; cy <= last_chunk.y; ++cy)
        {
            for (int cx = first_chunk.x; cx <= last_chunk.x; ++cx)
            {
                if (!ensure_chunk_generated(world, glm::ivec2(cx, cy)))
                    return false;
            }
        }

        return true;
    }

    bool is_valid_placement(StructureSpawner *spawner, World *world,
                            const Structure &structure, const glm::ivec2 &world_pos,
                            const std::string &structure_name)
    {
        if (!spawner || !world)
            return false;

        if (!ensure_footprint_chunks_generated(world, structure, world_pos))
            return false;

        const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();

        // Rule 0: structure footprint cannot overlap/touch another already placed structure.
        const int clearance_px = STRUCTURE_CLEARANCE_CELLS * ps;
        const int cand_left = world_pos.x - clearance_px;
        const int cand_top = world_pos.y - clearance_px;
        const int cand_right = world_pos.x + structure.get_width() * ps + clearance_px;
        const int cand_bottom = world_pos.y + structure.get_height() * ps + clearance_px;
        const int cand_left_nominal = world_pos.x;
        const int cand_top_nominal = world_pos.y;
        const int cand_right_nominal = world_pos.x + structure.get_width() * ps;
        const int cand_bottom_nominal = world_pos.y + structure.get_height() * ps;

        for (const auto &placed : spawner->get_placed_structures())
        {
            Structure *placed_blueprint = spawner->get_blueprint(placed.name);
            if (!placed_blueprint)
                continue;

            const int placed_left = placed.position.x;
            const int placed_top = placed.position.y;
            const int placed_right = placed.position.x + placed_blueprint->get_width() * ps;
            const int placed_bottom = placed.position.y + placed_blueprint->get_height() * ps;

            const bool separated =
                cand_right <= placed_left ||
                cand_left >= placed_right ||
                cand_bottom <= placed_top ||
                cand_top >= placed_bottom;

            if (!separated)
                return false;

            // Rule 0.5: prevent vertical stacking (one structure directly above/below another)
            // whenever their horizontal footprints overlap.
            const bool x_overlap =
                cand_right_nominal > placed_left &&
                cand_left_nominal < placed_right;

            if (x_overlap)
            {
                const bool sits_on_top =
                    cand_bottom_nominal >= placed_top - ps &&
                    cand_bottom_nominal <= placed_top + ps;

                const bool sits_below =
                    placed_bottom >= cand_top_nominal - ps &&
                    placed_bottom <= cand_top_nominal + ps;

                if (sits_on_top || sits_below)
                    return false;
            }
        }

        // Rule 1.1: every non-empty structure cell must map to empty world cell.
        for (int sy = 0; sy < structure.get_height(); ++sy)
        {
            for (int sx = 0; sx < structure.get_width(); ++sx)
            {
                const Particle &cell = structure.get_cell(sx, sy);
                if (cell.type == Particle_Type::EMPTY)
                    continue;

                const int world_x = world_pos.x + sx * ps;
                const int world_y = world_pos.y + sy * ps;

                Chunk *chunk = nullptr;
                glm::ivec2 local_cell;
                if (!get_chunk_and_local_cell(world, world_x, world_y, chunk, local_cell))
                    return false;

                if (!chunk->is_empty(local_cell.x, local_cell.y))
                    return false;
            }
        }

        // Rule 1.2: support and flatness rules are shared for all structures
        // and tuned via per-structure placement profiles.
        const StructureSpawner::PlacementProfile profile = spawner->resolve_structure_placement_profile(structure_name);
        const int ground_y = world_pos.y + structure.get_height() * ps;
        int supported_columns = 0;
        int min_support_depth = std::numeric_limits<int>::max();
        int max_support_depth = std::numeric_limits<int>::min();

        for (int sx = 0; sx < structure.get_width(); ++sx)
        {
            const int world_x = world_pos.x + sx * ps;
            int first_solid_depth = -1;

            for (int depth = 0; depth < profile.support_scan_max_depth_cells; ++depth)
            {
                const int sample_y = ground_y + depth * ps;

                const glm::ivec2 chunk_pos = get_chunk_pos_from_world_px(world_x,
                                                                         sample_y,
                                                                         chunk_dims.x * ps,
                                                                         chunk_dims.y * ps);
                if (!ensure_chunk_generated(world, chunk_pos))
                    return false;

                Chunk *chunk = nullptr;
                glm::ivec2 local_cell;
                if (!get_chunk_and_local_cell(world, world_x, sample_y, chunk, local_cell))
                    return false;

                if (!chunk->is_empty(local_cell.x, local_cell.y))
                {
                    first_solid_depth = depth;
                    break;
                }
            }

            if (first_solid_depth < 0)
                continue;

            supported_columns++;
            min_support_depth = std::min(min_support_depth, first_solid_depth);
            max_support_depth = std::max(max_support_depth, first_solid_depth);
        }

        const int required_supported_columns = std::min(std::max(1, profile.min_supported_columns), std::max(1, structure.get_width()));
        if (supported_columns < required_supported_columns)
            return false;

        const int structure_width = std::max(1, structure.get_width());
        const float support_ratio = static_cast<float>(supported_columns) / static_cast<float>(structure_width);
        if (support_ratio < profile.min_support_ratio)
            return false;

        if (supported_columns > 1)
        {
            const int variation = max_support_depth - min_support_depth;
            if (variation > profile.max_ground_variation_cells)
                return false;
        }

        return true;
    }

    bool try_place_with_vertical_lift(StructureSpawner *spawner, World *world,
                                      const Structure &blueprint, const glm::ivec2 &base_pos,
                                      const std::string &structure_name, int ps)
    {
        if (!spawner || !world)
            return false;

        for (int lift = 0; lift <= MAX_VERTICAL_LIFT_CELLS; ++lift)
        {
            glm::ivec2 candidate_pos(base_pos.x, base_pos.y - lift * ps);
            if (!is_valid_placement(spawner, world, blueprint, candidate_pos, structure_name))
                continue;

            if (spawner->place_structure(blueprint, candidate_pos))
            {
                spawner->record_placed_structure(candidate_pos, structure_name);
            }
            return true;
        }

        return false;
    }
}

int StructureSpawner::find_surface_y(int world_x, int start_y, int scan_range) const
{
    if (!world)
        return -1;

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
    const int chunk_pixel_w = chunk_dims.x * ps;
    const int chunk_pixel_h = chunk_dims.y * ps;

    for (int py = start_y; py < start_y + scan_range; py += ps)
    {
        glm::ivec2 chunk_pos = get_chunk_pos_from_world_px(world_x, py, chunk_pixel_w, chunk_pixel_h);
        Chunk *chunk = world->get_chunk(chunk_pos);
        if (!chunk)
            continue;

        int offset_x = world_x - chunk_pos.x * chunk_pixel_w;
        int offset_y = py - chunk_pos.y * chunk_pixel_h;
        int cell_x = offset_x / ps;
        int cell_y = offset_y / ps;
        if (cell_x < 0)
            cell_x += chunk_dims.x;
        if (cell_y < 0)
            cell_y += chunk_dims.y;

        if (cell_x < 0 || cell_x >= chunk_dims.x || cell_y < 0 || cell_y >= chunk_dims.y)
            continue;

        if (!chunk->is_empty(cell_x, cell_y))
            return py;
    }

    return -1;
}

void StructureSpawner::fill_structure_base(const Structure &structure, const glm::ivec2 &world_pos)
{
    if (!world)
        return;

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    const int ground_y = world_pos.y + structure.get_height() * ps;
    const int width = structure.get_width();
    if (width <= 0)
        return;

    std::vector<int> available_depth_by_column(width, 0);
    std::vector<int> filled_depth_by_column(width, 0);

    for (int sx = 0; sx < width; ++sx)
    {
        const int world_x = world_pos.x + sx * ps;

        for (int depth = 0; depth < GROUND_FILL_MAX_DEPTH_CELLS; ++depth)
        {
            const int fill_y = ground_y + depth * ps;
            Chunk *chunk = nullptr;
            glm::ivec2 local_cell;

            const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
            const glm::ivec2 chunk_pos = get_chunk_pos_from_world_px(world_x, fill_y, chunk_dims.x * ps, chunk_dims.y * ps);
            if (!ensure_chunk_generated(world, chunk_pos))
                break;

            if (!get_chunk_and_local_cell(world, world_x, fill_y, chunk, local_cell))
                break;

            if (!chunk->is_empty(local_cell.x, local_cell.y))
                break;

            available_depth_by_column[sx] = depth + 1;
        }
    }

    // Shape the support into a shallow bowl: deepest near center, shallower toward edges.
    const float center = 0.5f * static_cast<float>(width - 1);
    const float half_span = std::max(1.0f, 0.5f * static_cast<float>(width));
    for (int sx = 0; sx < width; ++sx)
    {
        const int available = available_depth_by_column[sx];
        if (available <= 0)
            continue;

        const float normalized = std::abs(static_cast<float>(sx) - center) / half_span;
        const float bowl = std::clamp(1.0f - normalized * normalized, 0.0f, 1.0f);
        const float keep_ratio = BASE_BOWL_EDGE_DEPTH_RATIO + (1.0f - BASE_BOWL_EDGE_DEPTH_RATIO) * bowl;

        int target_depth = static_cast<int>(std::ceil(static_cast<float>(available) * keep_ratio));
        target_depth = std::clamp(target_depth, 1, available);
        filled_depth_by_column[sx] = target_depth;
    }

    // Smooth adjacent columns to avoid staircase artifacts and keep a natural curve.
    for (int pass = 0; pass < 2; ++pass)
    {
        for (int sx = 1; sx < width; ++sx)
        {
            filled_depth_by_column[sx] = std::min(
                filled_depth_by_column[sx],
                filled_depth_by_column[sx - 1] + BASE_BOWL_SMOOTH_DELTA_CELLS);
        }

        for (int sx = width - 2; sx >= 0; --sx)
        {
            filled_depth_by_column[sx] = std::min(
                filled_depth_by_column[sx],
                filled_depth_by_column[sx + 1] + BASE_BOWL_SMOOTH_DELTA_CELLS);
        }
    }

    for (int sx = 0; sx < width; ++sx)
    {
        const int world_x = world_pos.x + sx * ps;
        const int target_depth = std::min(filled_depth_by_column[sx], available_depth_by_column[sx]);
        for (int depth = 0; depth < target_depth; ++depth)
        {
            const int fill_y = ground_y + depth * ps;
            Chunk *chunk = nullptr;
            glm::ivec2 local_cell;

            if (!get_chunk_and_local_cell(world, world_x, fill_y, chunk, local_cell))
                break;

            if (!chunk->is_empty(local_cell.x, local_cell.y))
                break;

            const Particle_Type fill_type = get_support_particle_type_for_world_px(world, world_x, fill_y);
            world->place_static_particle(glm::ivec2(world_x, fill_y), fill_type);
        }
    }

    // Feather support edges so the bowl transitions into terrain more naturally.
    const int half_width = width / 2;
    for (int sx = 0; sx < width; ++sx)
    {
        const int core_x = world_pos.x + sx * ps;
        int core_depth = filled_depth_by_column[sx];
        if (core_depth <= 0)
            continue;

        core_depth = std::min(core_depth, BASE_FEATHER_MAX_DEPTH_CELLS);

        for (int side = 1; side <= BASE_FEATHER_SIDE_SPREAD_CELLS; ++side)
        {
            const int left_x = core_x - side * ps;
            const int right_x = core_x + side * ps;
            const int tapered_depth = std::max(0, core_depth - side * 2 + 1);

            for (int depth = 0; depth < tapered_depth; ++depth)
            {
                const int fill_y = ground_y + depth * ps;

                const float depth_factor = static_cast<float>(depth) / static_cast<float>(BASE_FEATHER_MAX_DEPTH_CELLS);
                const float side_factor = static_cast<float>(side - 1) / static_cast<float>(BASE_FEATHER_SIDE_SPREAD_CELLS);
                const float keep_probability = std::clamp(0.75f - 0.30f * side_factor - 0.40f * depth_factor, 0.15f, 0.80f);

                const int salt = sx - half_width;

                auto try_fill = [&](int world_x)
                {
                    if (deterministic_noise_01(world_x, fill_y, salt) > keep_probability)
                        return;

                    const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
                    const glm::ivec2 chunk_pos = get_chunk_pos_from_world_px(world_x, fill_y, chunk_dims.x * ps, chunk_dims.y * ps);
                    if (!ensure_chunk_generated(world, chunk_pos))
                        return;

                    Chunk *chunk = nullptr;
                    glm::ivec2 local_cell;
                    if (!get_chunk_and_local_cell(world, world_x, fill_y, chunk, local_cell))
                        return;

                    if (!chunk->is_empty(local_cell.x, local_cell.y))
                        return;

                    const Particle_Type fill_type = get_support_particle_type_for_world_px(world, world_x, fill_y);
                    world->place_static_particle(glm::ivec2(world_x, fill_y), fill_type);
                };

                try_fill(left_x);
                try_fill(right_x);
            }
        }
    }
}

void StructureSpawner::generate_predetermined_positions(int world_seed)
{
    predetermined_entries.clear();
    placed_structures.clear();
    store_target_sequence_index = 0;
    store_regions.clear();
    store_region_queue.clear();
    store_region_queued.clear();
    runtime_stats = RuntimeStats{};
    runtime_stats_log_tick = 0;

    std::mt19937 local_rng(world_seed);
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * static_cast<float>(M_PI));
    std::uniform_int_distribution<int> random_pos(RANDOM_TARGET_MIN, RANDOM_TARGET_MAX);

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    const float devushki_spawn_radius_px = static_cast<float>(devushki_spawn_radius_particles * ps);
    for (const auto &pair : blueprints)
    {
        // Store targets are generated lazily from explored chunk count.
        if (is_store_name(pair.first))
            continue;

        int spawn_count = (pair.first == "devushki_column") ? 1 : NON_COLUMN_DEFAULT_SPAWN_OPPORTUNITIES;
        auto count_it = structure_spawn_counts.find(pair.first);
        if (count_it != structure_spawn_counts.end())
        {
            spawn_count = std::max(1, count_it->second);
        }

        for (int i = 0; i < spawn_count; ++i)
        {
            PredeterminedEntry entry;
            entry.structure_name = pair.first;

            if (pair.first == "devushki_column")
            {
                const float angle = angle_dist(local_rng);

                int target_x = static_cast<int>(std::cos(angle) * devushki_spawn_radius_px);
                int target_y = static_cast<int>(std::sin(angle) * devushki_spawn_radius_px);

                target_x = (target_x / ps) * ps;
                target_y = (target_y / ps) * ps;

                entry.target_pos = glm::ivec2(target_x, target_y);
            }
            else
            {
                int target_x = random_pos(local_rng);
                int target_y = random_pos(local_rng);
                target_x = (target_x / ps) * ps;
                target_y = (target_y / ps) * ps;

                entry.target_pos = glm::ivec2(target_x, target_y);
            }

            predetermined_entries.push_back(entry);
        }
    }
}

void StructureSpawner::try_place_pending_structures(const glm::ivec2 &chunk_coords, int max_entries_to_place)
{
    const auto profile_start = std::chrono::steady_clock::now();
    int scanned_entries = 0;
    int placed_entries = 0;
    int added_store_targets = 0;
    int generated_chunks = 0;
    int desired_store_targets = 0;
    int current_store_targets = 0;

    auto finalize_runtime_stats = [&](int pending_entries)
    {
        const auto profile_end = std::chrono::steady_clock::now();
        const double elapsed_ms = std::chrono::duration<double, std::milli>(profile_end - profile_start).count();

        runtime_stats.calls++;
        runtime_stats.last_call_ms = elapsed_ms;
        runtime_stats.avg_call_ms += (elapsed_ms - runtime_stats.avg_call_ms) / static_cast<double>(runtime_stats.calls);
        runtime_stats.max_call_ms = std::max(runtime_stats.max_call_ms, elapsed_ms);
        runtime_stats.scanned_entries_last = scanned_entries;
        runtime_stats.scanned_entries_avg +=
            (static_cast<double>(scanned_entries) - runtime_stats.scanned_entries_avg) / static_cast<double>(runtime_stats.calls);
        runtime_stats.placed_entries_last = placed_entries;
        runtime_stats.added_store_targets_last = added_store_targets;
        runtime_stats.generated_chunks_last = generated_chunks;
        runtime_stats.desired_store_targets_last = desired_store_targets;
        runtime_stats.current_store_targets_last = current_store_targets;
        runtime_stats.pending_entries_last = pending_entries;
        runtime_stats.total_predetermined_entries_last = static_cast<int>(predetermined_entries.size() + store_regions.size());

        if (!is_store_proof_logging_enabled())
            return;

        runtime_stats_log_tick++;
        if (runtime_stats_log_tick < 600)
            return;

        runtime_stats_log_tick = 0;
        std::ostringstream message;
        message << "store_prof"
                << " calls=" << runtime_stats.calls
                << " entries=" << runtime_stats.total_predetermined_entries_last
                << " pending=" << runtime_stats.pending_entries_last
                << " scanned_last=" << runtime_stats.scanned_entries_last
                << " scanned_avg=" << runtime_stats.scanned_entries_avg
                << " call_ms_last=" << runtime_stats.last_call_ms
                << " call_ms_avg=" << runtime_stats.avg_call_ms
                << " call_ms_max=" << runtime_stats.max_call_ms
                << " chunks=" << runtime_stats.generated_chunks_last
                << " stores_desired=" << runtime_stats.desired_store_targets_last
                << " stores_current=" << runtime_stats.current_store_targets_last
                << " placed_last=" << runtime_stats.placed_entries_last
                << " added_store_targets_last=" << runtime_stats.added_store_targets_last;
        Log::info(message.str());
    };

    if (!world)
    {
        finalize_runtime_stats(0);
        return;
    }

    const bool use_entry_budget = max_entries_to_place > 0;
    int processed_entries = 0;

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
    const int chunk_pixel_w = chunk_dims.x * ps;
    const int chunk_pixel_h = chunk_dims.y * ps;

    // Legacy migration guard: clear any old runtime-added store entries from the
    // global predetermined list. Store placement is now region-state driven.
    predetermined_entries.erase(
        std::remove_if(predetermined_entries.begin(), predetermined_entries.end(),
                       [&](const PredeterminedEntry &entry)
                       {
                           return is_store_name(entry.structure_name);
                       }),
        predetermined_entries.end());

    for (auto &entry : predetermined_entries)
    {
        scanned_entries++;

        if (use_entry_budget && processed_entries >= max_entries_to_place)
            break;

        if (entry.placed)
            continue;

        if (!store_spawn_config.enabled && is_store_name(entry.structure_name))
        {
            entry.placed = true;
            continue;
        }

        const glm::ivec2 target_chunk = get_chunk_pos_from_world_px(
            entry.target_pos.x,
            entry.target_pos.y,
            chunk_pixel_w,
            chunk_pixel_h);
        const int chunk_dx = std::abs(target_chunk.x - chunk_coords.x);
        const int chunk_dy = std::abs(target_chunk.y - chunk_coords.y);
        if (chunk_dx > ENTRY_PLACE_RADIUS_CHUNKS || chunk_dy > ENTRY_PLACE_RADIUS_CHUNKS)
            continue;

        Structure *blueprint = get_blueprint(entry.structure_name);
        if (!blueprint)
        {
            entry.placed = true;
            continue;
        }

        bool placed = false;
        glm::ivec2 raw_pos = entry.target_pos;

        std::uniform_int_distribution<int> retry_offset(-RAW_RETRY_DISTANCE_CELLS * ps,
                                                        RAW_RETRY_DISTANCE_CELLS * ps);

        for (int attempt = 0; attempt < MAX_RAW_RETRY_ATTEMPTS && !placed; ++attempt)
        {
            placed = try_place_with_vertical_lift(this, world, *blueprint, raw_pos, entry.structure_name, ps);
            if (placed)
                break;

            raw_pos.x += retry_offset(rng);
            raw_pos.y += retry_offset(rng);
            raw_pos.x = (raw_pos.x / ps) * ps;
            raw_pos.y = (raw_pos.y / ps) * ps;
        }

        if (placed)
        {
            entry.placed = true;
            entry.target_pos = raw_pos;
            processed_entries++;
            placed_entries++;
        }
    }

    Structure *store_blueprint = get_blueprint("store");
    if (store_spawn_config.enabled && store_blueprint)
    {
        generated_chunks = std::max(0, world->get_chunks_size());

        int spacing_chunks = store_spawn_config.spacing_chunks;
        if (spacing_chunks <= 0)
        {
            spacing_chunks = std::max(
                4,
                static_cast<int>(std::round(std::sqrt(static_cast<double>(std::max(1, store_spawn_config.chunks_per_spawn))))));
        }

        int separation_chunks = store_spawn_config.separation_chunks;
        if (separation_chunks <= 0)
            separation_chunks = std::max(1, spacing_chunks / 3);
        separation_chunks = std::clamp(separation_chunks, 0, std::max(0, spacing_chunks - 1));

        current_store_targets = 0;
        for (const auto &placed : placed_structures)
        {
            if (is_store_name(placed.name))
                current_store_targets++;
        }

        const int center_rx = floor_div_int(chunk_coords.x, spacing_chunks);
        const int center_ry = floor_div_int(chunk_coords.y, spacing_chunks);
        const int active_radius_chunks = std::max(1, store_spawn_config.active_region_radius_chunks);
        const int region_radius = std::max(1, (active_radius_chunks + spacing_chunks - 1) / spacing_chunks + 1);

        for (int ry = center_ry - region_radius; ry <= center_ry + region_radius; ++ry)
        {
            for (int rx = center_rx - region_radius; rx <= center_rx + region_radius; ++rx)
            {
                const glm::ivec2 region_key(rx, ry);
                auto region_insert = store_regions.emplace(region_key, StoreRegionData{});
                StoreRegionData &region = region_insert.first->second;

                if (!region.candidate_ready)
                {
                    const int local_span = std::max(1, spacing_chunks - separation_chunks);
                    std::uint64_t seed_mix = static_cast<std::uint64_t>(static_cast<std::uint32_t>(seed));
                    seed_mix ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(store_spawn_config.salt)) << 1;
                    seed_mix ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(rx)) * 0x9E3779B97F4A7C15ull;
                    seed_mix ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(ry)) * 0xBF58476D1CE4E5B9ull;

                    const std::uint64_t h1 = splitmix64(seed_mix);
                    const std::uint64_t h2 = splitmix64(h1 ^ 0x94D049BB133111EBull);

                    const int offset_x = static_cast<int>(h1 % static_cast<std::uint64_t>(local_span));
                    const int offset_y = static_cast<int>(h2 % static_cast<std::uint64_t>(local_span));

                    region.candidate_chunk = glm::ivec2(
                        rx * spacing_chunks + offset_x,
                        ry * spacing_chunks + offset_y);
                    region.candidate_pos = glm::ivec2(
                        region.candidate_chunk.x * chunk_pixel_w + chunk_pixel_w / 2,
                        region.candidate_chunk.y * chunk_pixel_h + chunk_pixel_h / 2);
                    region.candidate_ready = true;
                    added_store_targets++;
                }

                if (generated_chunks < store_spawn_config.min_generated_chunks_before_first_store)
                    continue;

                if (region.placed || region.exhausted)
                    continue;

                if (generated_chunks < region.retry_after_generated_chunks)
                    continue;

                if (store_region_queued.find(region_key) == store_region_queued.end())
                {
                    store_region_queue.push_back(region_key);
                    store_region_queued.insert(region_key);
                }
            }
        }

        desired_store_targets = static_cast<int>(store_regions.size());

        const int region_budget = std::max(1, store_spawn_config.max_regions_processed_per_call);
        int processed_regions = 0;

        while (!store_region_queue.empty() && processed_regions < region_budget)
        {
            const glm::ivec2 region_key = store_region_queue.front();
            store_region_queue.pop_front();
            store_region_queued.erase(region_key);

            auto region_it = store_regions.find(region_key);
            if (region_it == store_regions.end())
                continue;

            StoreRegionData &region = region_it->second;
            if (!region.candidate_ready || region.placed || region.exhausted)
                continue;

            if (generated_chunks < region.retry_after_generated_chunks)
                continue;

            const int chunk_dx = std::abs(region.candidate_chunk.x - chunk_coords.x);
            const int chunk_dy = std::abs(region.candidate_chunk.y - chunk_coords.y);
            if (chunk_dx > ENTRY_PLACE_RADIUS_CHUNKS || chunk_dy > ENTRY_PLACE_RADIUS_CHUNKS)
                continue;

            glm::ivec2 raw_pos = region.candidate_pos;

            const int jitter_cells = 24;
            const int jitter_max_px = jitter_cells * ps;
            std::uint64_t attempt_seed = static_cast<std::uint64_t>(static_cast<std::uint32_t>(seed));
            attempt_seed ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(store_spawn_config.salt)) << 7;
            attempt_seed ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(region_key.x)) * 0xD6E8FEB86659FD93ull;
            attempt_seed ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(region_key.y)) * 0xA5A3564E27F886EDull;
            attempt_seed ^= static_cast<std::uint64_t>(region.attempts + 1) * 0x9E3779B97F4A7C15ull;

            const std::uint64_t ah1 = splitmix64(attempt_seed);
            const std::uint64_t ah2 = splitmix64(ah1 ^ 0x9E3779B97F4A7C15ull);

            const int jitter_x = static_cast<int>(ah1 % static_cast<std::uint64_t>(2 * jitter_max_px + 1)) - jitter_max_px;
            const int jitter_y = static_cast<int>(ah2 % static_cast<std::uint64_t>(2 * jitter_max_px + 1)) - jitter_max_px;
            raw_pos.x += jitter_x;
            raw_pos.y += jitter_y;
            raw_pos.x = floor_div_int(raw_pos.x, ps) * ps;
            raw_pos.y = floor_div_int(raw_pos.y, ps) * ps;

            bool placed = false;
            if (is_store_target_far_enough(raw_pos))
            {
                placed = try_place_with_vertical_lift(this, world, *store_blueprint, raw_pos, "store", ps);
            }

            if (placed)
            {
                region.placed = true;
                placed_entries++;
                current_store_targets++;
            }
            else
            {
                region.attempts++;
                if (region.attempts >= store_spawn_config.max_attempts_per_region)
                {
                    region.exhausted = true;
                }
                else
                {
                    region.retry_after_generated_chunks =
                        generated_chunks + store_spawn_config.retry_generated_chunks;
                }
            }

            processed_regions++;
        }
    }

    int pending_entries = 0;
    for (const auto &entry : predetermined_entries)
    {
        if (!entry.placed)
            pending_entries++;
    }

    for (const auto &[region_key, region] : store_regions)
    {
        (void)region_key;
        if (!region.placed && !region.exhausted)
            pending_entries++;
    }

    finalize_runtime_stats(pending_entries);
}

void StructureSpawner::place_pending_for_structure(const std::string &name, int max_passes)
{
    if (!world)
        return;

    max_passes = std::max(1, max_passes);

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);

    for (int pass = 0; pass < max_passes; ++pass)
    {
        bool has_pending = false;

        for (auto &entry : predetermined_entries)
        {
            if (entry.placed || entry.structure_name != name)
                continue;

            has_pending = true;

            Structure *blueprint = get_blueprint(entry.structure_name);
            if (!blueprint)
            {
                entry.placed = true;
                continue;
            }

            bool placed = false;
            glm::ivec2 raw_pos = entry.target_pos;

            std::uniform_int_distribution<int> retry_offset(-RAW_RETRY_DISTANCE_CELLS * ps,
                                                            RAW_RETRY_DISTANCE_CELLS * ps);

            for (int attempt = 0; attempt < MAX_RAW_RETRY_ATTEMPTS && !placed; ++attempt)
            {
                placed = try_place_with_vertical_lift(this, world, *blueprint, raw_pos, entry.structure_name, ps);
                if (placed)
                    break;

                raw_pos.x += retry_offset(rng);
                raw_pos.y += retry_offset(rng);
                raw_pos.x = (raw_pos.x / ps) * ps;
                raw_pos.y = (raw_pos.y / ps) * ps;
            }

            if (placed)
            {
                entry.placed = true;
                entry.target_pos = raw_pos;
            }
        }

        if (!has_pending)
            break;
    }
}

void StructureSpawner::prepare_pending_targets_for_structure(
    const std::string &name,
    int max_passes,
    const std::function<void(const std::string &, float)> &progress_callback)
{
    if (!world)
        return;

    max_passes = std::max(1, max_passes);

    std::unordered_map<std::size_t, int> entry_ordinals;
    int ordinal_counter = 0;
    int pending_target_count = 0;

    for (std::size_t i = 0; i < predetermined_entries.size(); ++i)
    {
        const auto &entry = predetermined_entries[i];
        if (entry.structure_name != name)
            continue;

        ordinal_counter++;
        entry_ordinals[i] = ordinal_counter;
        if (!entry.placed)
            pending_target_count++;
    }

    const int total_attempts = std::max(1, pending_target_count * max_passes);
    int attempts_done = 0;

    auto report_progress = [&](const std::string &status, float progress)
    {
        if (progress_callback)
            progress_callback(status, std::clamp(progress, 0.0f, 1.0f));
    };

    if (pending_target_count == 0)
    {
        report_progress("No pending targets to prepare.", 1.0f);
        return;
    }

    report_progress("Preparing structure targets...", 0.0f);

    std::vector<ReservedPlacement> reserved;
    reserved.reserve(placed_structures.size() + predetermined_entries.size());

    for (const auto &placed : placed_structures)
    {
        Structure *placed_blueprint = get_blueprint(placed.name);
        if (!placed_blueprint)
            continue;
        reserved.push_back({placed.position, placed_blueprint});
    }

    for (int pass = 0; pass < max_passes; ++pass)
    {
        bool has_pending = false;

        for (std::size_t entry_index = 0; entry_index < predetermined_entries.size(); ++entry_index)
        {
            auto &entry = predetermined_entries[entry_index];
            if (entry.placed || entry.structure_name != name)
                continue;

            has_pending = true;

            const int ordinal = entry_ordinals.count(entry_index) ? entry_ordinals[entry_index] : static_cast<int>(entry_index + 1);
            const float attempt_start_progress = static_cast<float>(attempts_done) / static_cast<float>(total_attempts);

            if (name == "devushki_column")
            {
                report_progress(
                    "Trying to find coords for column " + std::to_string(ordinal) +
                        " (pass " + std::to_string(pass + 1) + "/" + std::to_string(max_passes) + ")",
                    attempt_start_progress);
            }
            else
            {
                report_progress(
                    "Resolving target for " + name + " #" + std::to_string(ordinal),
                    attempt_start_progress);
            }

            attempts_done++;

            Structure *blueprint = get_blueprint(entry.structure_name);
            if (!blueprint)
            {
                entry.placed = true;
                continue;
            }

            glm::ivec2 resolved_target;
            const bool found_target = resolve_valid_target_for_entry(
                world,
                *blueprint,
                entry.target_pos,
                entry_index,
                pass,
                reserved,
                resolved_target);

            if (!found_target)
            {
                report_progress(
                    name == "devushki_column"
                        ? ("Could not resolve column " + std::to_string(ordinal) + " in this pass")
                        : ("Could not resolve target for " + name + " #" + std::to_string(ordinal)),
                    static_cast<float>(attempts_done) / static_cast<float>(total_attempts));
                continue;
            }

            entry.target_pos = resolved_target;
            reserved.push_back({resolved_target, blueprint});

            if (name == "devushki_column")
            {
                report_progress(
                    "Found coords for column " + std::to_string(ordinal) +
                        ": (" + std::to_string(resolved_target.x) + ", " + std::to_string(resolved_target.y) + ")",
                    static_cast<float>(attempts_done) / static_cast<float>(total_attempts));
            }
            else
            {
                report_progress(
                    "Resolved target for " + name + " #" + std::to_string(ordinal),
                    static_cast<float>(attempts_done) / static_cast<float>(total_attempts));
            }
        }

        if (!has_pending)
            break;
    }

    report_progress("Target preparation complete.", 1.0f);
}

const std::vector<StructureSpawner::PredeterminedEntry> &StructureSpawner::get_predetermined_entries() const
{
    return predetermined_entries;
}

void StructureSpawner::record_placed_structure(const glm::ivec2 &position, const std::string &name)
{
    placed_structures.push_back({position, name});
}

const std::vector<StructureSpawner::PlacedStructure> &StructureSpawner::get_placed_structures() const
{
    return placed_structures;
}

StructureSpawner::RuntimeStats StructureSpawner::get_runtime_stats() const
{
    return runtime_stats;
}

// ============================================
// ImageStructureLoader
// ============================================

bool ImageStructureLoader::is_pixel_empty(int r, int g, int b, int a)
{
    return a < 10;
}

float ImageStructureLoader::color_distance(int r1, int g1, int b1, int r2, int g2, int b2)
{
    float dr = static_cast<float>(r1 - r2);
    float dg = static_cast<float>(g1 - g2);
    float db = static_cast<float>(b1 - b2);
    return std::sqrt(dr * dr + dg * dg + db * db);
}

Particle ImageStructureLoader::pixel_to_particle(int r, int g, int b, int a)
{
    if (is_pixel_empty(r, g, b, a))
        return Particle(); // EMPTY

    // Exact match lookup (0–255 RGB)
    if (r == 194 && g == 178 && b == 128)
        return create_sand(true);
    if (r == 64 && g == 164 && b == 223)
        return create_water(true);
    if (r == 128 && g == 128 && b == 128)
        return create_stone(true);
    if (r == 200 && g == 200 && b == 200)
        return create_smoke(true);
    if (r == 139 && g == 94 && b == 60)
        return create_wood(true);
    if (r == 255 && g == 110 && b == 30)
        return create_fire(true);
    if (r == 0 && g == 255 && b == 70)
        return create_uranium(true);

    // No exact match — create a default immovable stone-like particle with the pixel's color
    Particle p(
        Particle_Type::STONE,
        Particle_State::SOLID,
        Particle_Movement::STATIC,
        Color(r, g, b, 1.0f));
    p.set_static(true);
    return p;
}

std::array<int, 4> ImageStructureLoader::find_content_bounds(const unsigned char *data,
                                                             int width, int height, int channels)
{
    int min_x = width;
    int min_y = height;
    int max_x = -1;
    int max_y = -1;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int idx = (y * width + x) * channels;
            int a = (channels >= 4) ? data[idx + 3] : 255;

            if (!is_pixel_empty(data[idx], data[idx + 1], data[idx + 2], a))
            {
                if (x < min_x)
                    min_x = x;
                if (y < min_y)
                    min_y = y;
                if (x > max_x)
                    max_x = x;
                if (y > max_y)
                    max_y = y;
            }
        }
    }

    // If no non-transparent pixels found, return zero-size bounds
    if (max_x < 0)
        return {0, 0, 0, 0};

    return {min_x, min_y, max_x, max_y};
}

Structure ImageStructureLoader::load_from_image(const std::string &image_path)
{
    int img_width, img_height, channels;
    unsigned char *data = stbi_load(image_path.c_str(), &img_width, &img_height, &channels, 4);

    if (!data)
    {
        std::cerr << "Failed to load structure image: " << image_path << std::endl;
        return Structure("error", 0, 0);
    }

    // Always 4 channels (RGBA) since we requested 4
    channels = 4;

    auto bounds = find_content_bounds(data, img_width, img_height, channels);
    int min_x = bounds[0];
    int min_y = bounds[1];
    int max_x = bounds[2];
    int max_y = bounds[3];

    int struct_width = max_x - min_x + 1;
    int struct_height = max_y - min_y + 1;

    if (struct_width <= 0 || struct_height <= 0)
    {
        stbi_image_free(data);
        return Structure("empty", 0, 0);
    }

    // Extract name from filename (without extension)
    std::filesystem::path filepath(image_path);
    std::string name = filepath.stem().string();

    Structure structure(name, struct_width, struct_height);

    for (int y = min_y; y <= max_y; y++)
    {
        for (int x = min_x; x <= max_x; x++)
        {
            int idx = (y * img_width + x) * channels;
            int r = data[idx];
            int g = data[idx + 1];
            int b = data[idx + 2];
            int a = data[idx + 3];

            Particle p = pixel_to_particle(r, g, b, a);
            structure.set_cell(x - min_x, y - min_y, p);
        }
    }

    stbi_image_free(data);
    return structure;
}

std::map<std::string, Structure> ImageStructureLoader::load_all_from_folder(const std::string &folder_path)
{
    std::map<std::string, Structure> structures;

    if (!std::filesystem::exists(folder_path) || !std::filesystem::is_directory(folder_path))
    {
        std::cerr << "Structure folder not found: " << folder_path << std::endl;
        return structures;
    }

    for (const auto &entry : std::filesystem::directory_iterator(folder_path))
    {
        if (!entry.is_regular_file())
            continue;

        std::string ext = entry.path().extension().string();
        // Convert to lowercase for comparison
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext != ".png")
            continue;

        Structure s = load_from_image(entry.path().string());
        if (s.get_width() > 0 && s.get_height() > 0)
        {
            structures[s.get_name()] = std::move(s);
        }
    }

    return structures;
}
