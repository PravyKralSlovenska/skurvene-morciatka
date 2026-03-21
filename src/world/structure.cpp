#include "engine/world/structure.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/world/world_ca_generation.hpp"
#include "others/GLOBALS.hpp"
#include "others/utils.hpp"

#include "stb/stb_image.h"

#include <filesystem>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <climits>
#include <memory>
#include <cstdint>

static int pixel_to_cell(int px)
{
    return static_cast<int>(std::floor(static_cast<float>(px) / Globals::PARTICLE_SIZE));
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
    static constexpr float DEVUSHKI_SPAWN_RADIUS = 1000.0f;
    static constexpr float STORE_SPAWN_RADIUS_MIN = 250.0f;
    static constexpr float STORE_SPAWN_RADIUS_MAX = 800.0f;
    static constexpr int MAX_VERTICAL_LIFT_CELLS = 120;
    static constexpr int MAX_RAW_RETRY_ATTEMPTS = 6;
    static constexpr int RAW_RETRY_DISTANCE_CELLS = 250;
    static constexpr int GROUND_FILL_MAX_DEPTH_CELLS = 512;
    static constexpr int BASE_FEATHER_SIDE_SPREAD_CELLS = 2;
    static constexpr int BASE_FEATHER_MAX_DEPTH_CELLS = 24;
    static constexpr int STRUCTURE_CLEARANCE_CELLS = 1;
    static constexpr int STORE_CHUNKS_PER_SPAWN = 100;
    static constexpr int ENTRY_PLACE_RADIUS_CHUNKS = 10;

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
                            const Structure &structure, const glm::ivec2 &world_pos)
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

        // Rule 1.2: at least one support column must touch solid ground.
        // Remaining unsupported columns can be stabilized by filling after placement.
        const int ground_y = world_pos.y + structure.get_height() * ps;
        int supported_columns = 0;
        for (int sx = 0; sx < structure.get_width(); ++sx)
        {
            const int world_x = world_pos.x + sx * ps;

            const glm::ivec2 chunk_pos = get_chunk_pos_from_world_px(world_x, ground_y, chunk_dims.x * ps, chunk_dims.y * ps);
            if (!ensure_chunk_generated(world, chunk_pos))
                return false;

            Chunk *chunk = nullptr;
            glm::ivec2 local_cell;
            if (!get_chunk_and_local_cell(world, world_x, ground_y, chunk, local_cell))
                return false;

            if (!chunk->is_empty(local_cell.x, local_cell.y))
                supported_columns++;
        }

        if (supported_columns == 0)
            return false;

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
            if (!is_valid_placement(spawner, world, blueprint, candidate_pos))
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
    std::vector<int> filled_depth_by_column(structure.get_width(), 0);

    for (int sx = 0; sx < structure.get_width(); ++sx)
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

            world->place_static_particle(glm::ivec2(world_x, fill_y), Particle_Type::STONE);
            filled_depth_by_column[sx] = depth + 1;
        }
    }

    // Feather support edges so the base transitions into terrain more naturally,
    // while keeping the full vertical support directly under the structure.
    const int half_width = structure.get_width() / 2;
    for (int sx = 0; sx < structure.get_width(); ++sx)
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
            const int tapered_depth = std::max(0, core_depth - side + 1);

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

                    world->place_static_particle(glm::ivec2(world_x, fill_y), Particle_Type::STONE);
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

    std::mt19937 local_rng(world_seed);
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * static_cast<float>(M_PI));
    std::uniform_real_distribution<float> store_radius_dist(STORE_SPAWN_RADIUS_MIN, STORE_SPAWN_RADIUS_MAX);
    std::uniform_int_distribution<int> random_pos(RANDOM_TARGET_MIN, RANDOM_TARGET_MAX);

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    for (const auto &pair : blueprints)
    {
        int spawn_count = 1;
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

                int target_x = static_cast<int>(std::cos(angle) * DEVUSHKI_SPAWN_RADIUS);
                int target_y = static_cast<int>(std::sin(angle) * DEVUSHKI_SPAWN_RADIUS);

                target_x = (target_x / ps) * ps;
                target_y = (target_y / ps) * ps;

                entry.target_pos = glm::ivec2(target_x, target_y);
            }
            else if (pair.first == "store" || pair.first == "devushki_store")
            {
                const float angle = angle_dist(local_rng);
                const float radius = store_radius_dist(local_rng);

                int target_x = static_cast<int>(std::cos(angle) * radius);
                int target_y = static_cast<int>(std::sin(angle) * radius);

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

void StructureSpawner::try_place_pending_structures(const glm::ivec2 &chunk_coords)
{
    if (!world)
        return;

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
    const int chunk_pixel_w = chunk_dims.x * ps;
    const int chunk_pixel_h = chunk_dims.y * ps;

    // Scale store count with explored world size: ~1 store per 100 generated chunks.
    Structure *store_blueprint = get_blueprint("store");
    if (store_blueprint)
    {
        const int generated_chunks = std::max(0, world->get_chunks_size());
        const int desired_store_targets = std::max(1, generated_chunks / STORE_CHUNKS_PER_SPAWN);

        int current_store_targets = 0;
        for (const auto &entry : predetermined_entries)
        {
            if (entry.structure_name == "store" || entry.structure_name == "devushki_store")
                ++current_store_targets;
        }

        // Add at most one store target per call to avoid frame spikes.
        if (current_store_targets < desired_store_targets)
        {
            std::uniform_int_distribution<int> random_pos(RANDOM_TARGET_MIN, RANDOM_TARGET_MAX);

            PredeterminedEntry entry;
            entry.structure_name = "store";

            int target_x = random_pos(rng);
            int target_y = random_pos(rng);
            target_x = (target_x / ps) * ps;
            target_y = (target_y / ps) * ps;

            entry.target_pos = glm::ivec2(target_x, target_y);
            predetermined_entries.push_back(entry);
        }
    }

    for (auto &entry : predetermined_entries)
    {
        if (entry.placed)
            continue;

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
        }
    }
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
