#include "engine/world/world.hpp"

#include "engine/player/entity.hpp"
#include "engine/particle/particle_movement.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/world/world_ca_generation.hpp"
#include "engine/world/herringbone_world_generation.hpp"
#include "engine/particle/particle.hpp"
#include "engine/particle/falling_sand_simulation.hpp"

#include <random>
#include <algorithm>
#include <iostream>
#include <climits>

World::World()
{
    world_gen = std::make_unique<World_CA_Generation>(chunk_width, chunk_height);
    world_gen->set_seed(1);

    // Initialize falling sand simulation
    sand_simulation = std::make_unique<Falling_Sand_Simulation>();
    sand_simulation->set_world(this);

    // Initialize structure spawner
    structure_spawner.set_world(this);
    structure_spawner.set_seed(1);
    structure_spawner.setup_default_rules();

    // Load image-based structures and generate predetermined spawn positions
    load_image_structures("../structure_images");

    generate_predetermined_positions();
}

World::~World() = default;

void World::update()
{
    update(0.016f); // Default to ~60fps delta time
}

void World::update(float delta_time)
{
    calculate_active_chunks();
    // update_active_chunks();

    // Run falling sand simulation
    if (simulation_enabled && sand_simulation)
    {
        sand_simulation->update(delta_time);
    }
}

void World::enable_simulation(bool enabled)
{
    simulation_enabled = enabled;
}

bool World::is_simulation_enabled() const
{
    return simulation_enabled;
}

Falling_Sand_Simulation *World::get_simulation()
{
    return sand_simulation.get();
}

void World::update_active_chunks()
{
    for (const auto &coords : active_chunks)
    {
        auto chunk = get_chunk(coords);

        if (chunk->get_state() != Chunk_States::FINALL_FORM)
        {
            // world_gen->iteration(chunk);
            iterate(chunk);
        }
    }
}

void World::set_player(Player *player)
{
    this->player = player;
}

inline int World::get_index(int x, int y)
{
    return y * width + x;
}

int World::change_width(int n)
{
    width += n;
    return width;
}

int World::change_height(int n)
{
    height += n;
    return height;
}

std::unique_ptr<Chunk> World::create_chunk(const int x, const int y)
{
    return create_chunk(glm::ivec2{x, y});
}

std::unique_ptr<Chunk> World::create_chunk(const glm::ivec2 &coords)
{
    std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(coords, chunk_width, chunk_height);

    world_gen->generate_chunk_with_biome(chunk.get());

    // world_gen->carve_cave_noise(chunk.get());

    return chunk;
}

void World::add_chunk(int x, int y)
{
    add_chunk(glm::ivec2{x, y});
}

void World::add_chunk(glm::ivec2 coords)
{
    if (world.find(coords) == world.end())
    {
        world.emplace(coords, create_chunk(coords));

        // Try to spawn structures in the newly created chunk
        structure_spawner.try_spawn_in_chunk(coords, chunk_width, chunk_height);

        // Check if any predetermined structure positions fall in this chunk
        try_place_predetermined_structures(coords);
    }
}

void World::calculate_active_chunks()
{
    active_chunks.clear();

    std::vector<glm::ivec2> offsets = calculate_offsets(chunk_radius);
    // std::vector<glm::ivec2> offsets = calculate_offsets_rectangle(5, 3);
    active_chunks.reserve(offsets.size());

    for (const auto &offset : offsets)
    {
        int chunk_pixel_width = chunk_width * Globals::PARTICLE_SIZE;
        int chunk_pixel_height = chunk_height * Globals::PARTICLE_SIZE;

        int x = floor(player->coords.x / chunk_pixel_width) + offset.x;
        int y = floor(player->coords.y / chunk_pixel_height) + offset.y;

        glm::ivec2 coords{x, y};

        add_chunk(coords);
        // world[coords]->set_state(Chunk_States::LOADED);

        // std::cout << x << ';' << y << '\n';
        active_chunks.insert(coords);
    }

    // std::cout << active_chunks.size() << '\n';
    // std::cout << world.size() << '\n';
}

static void place_particle_internal(World *world, const glm::ivec2 position,
                                    const Particle_Type particle_type, bool is_static,
                                    int chunk_width, int chunk_height)
{
    int chunk_pixel_width = chunk_width * Globals::PARTICLE_SIZE;
    int chunk_pixel_height = chunk_height * Globals::PARTICLE_SIZE;

    glm::ivec2 chunk_pos{
        (int)floor((float)(position.x) / chunk_pixel_width),
        (int)floor((float)(position.y) / chunk_pixel_height)};

    Chunk *chunk = world->get_chunk(chunk_pos);
    if (!chunk)
    {
        std::cerr << "nullptr chunk: " << chunk_pos.x << ';' << chunk_pos.y << '\n';
        return;
    }

    int pixel_offset_x = position.x - (chunk_pos.x * chunk_pixel_width);
    int pixel_offset_y = position.y - (chunk_pos.y * chunk_pixel_height);

    glm::ivec2 worldcell_pos{
        pixel_offset_x / Globals::PARTICLE_SIZE,
        pixel_offset_y / Globals::PARTICLE_SIZE};

    if (worldcell_pos.x < 0)
        worldcell_pos.x += chunk_width;
    if (worldcell_pos.y < 0)
        worldcell_pos.y += chunk_height;

    if (!in_world_range(worldcell_pos.x, worldcell_pos.y, chunk_height, chunk_width))
    {
        std::cerr << "suradnice su mimo: " << worldcell_pos.x << ';' << worldcell_pos.y << '\n';
        return;
    }

    chunk->set_worldcell(worldcell_pos, particle_type, is_static);
}

void World::place_particle(const glm::ivec2 position, const Particle_Type particle_type)
{
    // Player-placed particles are NOT static (they can fall/flow)
    place_particle_internal(this, position, particle_type, false, chunk_width, chunk_height);
}

void World::place_static_particle(const glm::ivec2 position, const Particle_Type particle_type)
{
    // Static particles don't move (for terrain building)
    place_particle_internal(this, position, particle_type, true, chunk_width, chunk_height);
}

void World::place_custom_particle(const glm::ivec2 position, const Particle &particle)
{
    int chunk_pixel_width = chunk_width * Globals::PARTICLE_SIZE;
    int chunk_pixel_height = chunk_height * Globals::PARTICLE_SIZE;

    glm::ivec2 chunk_pos{
        (int)floor((float)(position.x) / chunk_pixel_width),
        (int)floor((float)(position.y) / chunk_pixel_height)};

    Chunk *chunk = get_chunk(chunk_pos);
    if (!chunk)
    {
        std::cerr << "nullptr chunk (custom particle): " << chunk_pos.x << ';' << chunk_pos.y << '\n';
        return;
    }

    int pixel_offset_x = position.x - (chunk_pos.x * chunk_pixel_width);
    int pixel_offset_y = position.y - (chunk_pos.y * chunk_pixel_height);

    glm::ivec2 worldcell_pos{
        pixel_offset_x / Globals::PARTICLE_SIZE,
        pixel_offset_y / Globals::PARTICLE_SIZE};

    if (worldcell_pos.x < 0)
        worldcell_pos.x += chunk_width;
    if (worldcell_pos.y < 0)
        worldcell_pos.y += chunk_height;

    if (!in_world_range(worldcell_pos.x, worldcell_pos.y, chunk_height, chunk_width))
    {
        std::cerr << "suradnice su mimo (custom particle): " << worldcell_pos.x << ';' << worldcell_pos.y << '\n';
        return;
    }

    chunk->set_worldcell(worldcell_pos, particle);
}

void World::iterate(Chunk *chunk)
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
            if (offset.x == 0 && offset.y == 0)
                continue;

            glm::ivec2 neighbor_coords = cell.coords + offset;

            if (in_world_range(neighbor_coords.x, neighbor_coords.y, chunk->height, chunk->width))
            {
                auto neighbor = old_chunk_data->at(get_index_custom(neighbor_coords.x, neighbor_coords.y, chunk_width));
                if (neighbor.particle.type != Particle_Type::EMPTY)
                {
                    solid_neighbors++;
                }
            }
            // else
            // {
            //     glm::ivec2 neighboring_chunk_coords_offset{0, 0};
            //     glm::ivec2 neighboring_chunk_cell_coords = neighbor_coords;

            //     if (neighbor_coords.x >= chunk_width)
            //     {
            //         neighboring_chunk_coords_offset.x = 1;
            //         neighboring_chunk_cell_coords.x = 0;
            //     }
            //     else if (neighbor_coords.x < 0)
            //     {
            //         neighboring_chunk_coords_offset.x = -1;
            //         neighboring_chunk_cell_coords.x = chunk_width - 1;
            //     }

            //     if (neighbor_coords.y >= chunk_height)
            //     {
            //         neighboring_chunk_coords_offset.y = 1;
            //         neighboring_chunk_cell_coords.y = 0;
            //     }
            //     else if (neighbor_coords.y < 0)
            //     {
            //         neighboring_chunk_coords_offset.y = -1;
            //         neighboring_chunk_cell_coords.y = chunk_height - 1;
            //     }

            //     Chunk *neighboring_chunk = get_chunk(chunk->coords + neighboring_chunk_coords_offset);
            //     if (neighboring_chunk)
            //     {
            //         auto neighbor = neighboring_chunk->get_if_not_empty(
            //             neighboring_chunk_cell_coords.x,
            //             neighboring_chunk_cell_coords.y);

            //         if (neighbor)
            //         {
            //             solid_neighbors++;
            //         }
            //     }
            //     else
            //     {
            //         solid_neighbors++;
            //     }
            // }
        }

        bool is_currently_solid = (cell.particle.type != Particle_Type::EMPTY);

        if (is_currently_solid)
        {
            if (solid_neighbors >= 4)
            {
                new_chunk_data.emplace_back(cell.coords, create_stone());
            }
            else
            {
                new_chunk_data.emplace_back(cell.coords);
            }
        }
        else
        {
            if (solid_neighbors >= 5)
            {
                new_chunk_data.emplace_back(cell.coords, create_stone());
            }
            else
            {
                new_chunk_data.emplace_back(cell.coords);
            }
        }
    }

    if (new_chunk_data == *old_chunk_data)
    {
        chunk->set_state(Chunk_States::FINALL_FORM);
    }

    chunk->set_chunk_data(new_chunk_data);
}

std::vector<WorldCell *> World::find_solid_neighbors(WorldCell *cell, Chunk *chunk)
{
    std::vector<WorldCell *> neighbors;
    std::vector<glm::ivec2> neighbors_offsets = calculate_offsets_square(1);

    for (const auto &offset : neighbors_offsets)
    {
        if (offset.x == 0 && offset.y == 0)
        {
            continue;
        }

        glm::ivec2 neighbor_coords = cell->coords + offset;
        WorldCell *neighbor = nullptr;

        if (in_world_range(neighbor_coords.x, neighbor_coords.y, chunk_height, chunk_width))
        {
            neighbor = chunk->get_if_not_empty(neighbor_coords.x, neighbor_coords.y);
        }
        else
        {
            glm::ivec2 neighboring_chunk_coords = chunk->coords;
            glm::ivec2 local_coords = neighbor_coords;

            if (neighbor_coords.x < 0)
            {
                neighboring_chunk_coords.x -= 1;
                local_coords.x = chunk_width - 1;
            }
            else if (neighbor_coords.x >= chunk_width)
            {
                neighboring_chunk_coords.x += 1;
                local_coords.x = 0;
            }

            if (neighbor_coords.y < 0)
            {
                neighboring_chunk_coords.y -= 1;
                local_coords.y = chunk_height - 1;
            }
            else if (neighbor_coords.y >= chunk_height)
            {
                neighboring_chunk_coords.y += 1;
                local_coords.y = 0;
            }

            Chunk *neighboring_chunk = get_chunk(neighboring_chunk_coords);
            if (neighboring_chunk)
            {
                neighbor = neighboring_chunk->get_if_not_empty(
                    local_coords.x,
                    local_coords.y);
            }
        }

        if (neighbor)
        {
            neighbors.push_back(neighbor);
        }
    }

    return neighbors;
}

glm::ivec2 World::get_chunk_dimensions()
{
    return glm::ivec2(chunk_width, chunk_height);
}

std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> *World::get_chunks()
{
    return &world;
}

int World::get_chunks_size()
{
    return world.size();
}

std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> *World::get_active_chunks()
{
    return &active_chunks;
}

Chunk *World::get_chunk(const int x, const int y)
{
    return get_chunk(glm::ivec2{x, y});
}

Chunk *World::get_chunk(const glm::ivec2 &coords)
{
    auto it = world.find(coords);
    if (it == world.end())
    {
        return nullptr;
    }

    return it->second.get();
}

StructureSpawner &World::get_structure_spawner()
{
    return structure_spawner;
}

void World::place_structure(const Structure &structure, const glm::ivec2 &world_pos)
{
    structure_spawner.place_structure(structure, world_pos);
}

void World::place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos)
{
    structure_spawner.place_structure_centered(structure, center_pos);
}

void World::load_image_structures(const std::string &folder_path)
{
    image_structures = ImageStructureLoader::load_all_from_folder(folder_path);
}

const std::map<std::string, Structure> &World::get_image_structures() const
{
    return image_structures;
}

Structure *World::get_image_structure(const std::string &name)
{
    auto it = image_structures.find(name);
    if (it != image_structures.end())
        return &it->second;
    return nullptr;
}

// ============================================
// Predetermined structure spawn system
// ============================================

void World::generate_predetermined_positions()
{
    predetermined_spawn_positions.clear();
    pending_predetermined_positions.clear();

    constexpr float MIN_PREDETERMINED_DISTANCE = 800.0f;

    std::mt19937 rng(static_cast<unsigned>(1)); // world seed

    int cpw = static_cast<int>(chunk_width * Globals::PARTICLE_SIZE);
    int cph = static_cast<int>(chunk_height * Globals::PARTICLE_SIZE);

    // Playable area in pixels: chunk_radius chunks in each direction from origin
    int min_px = -chunk_radius * cpw;
    int max_px = chunk_radius * cpw;
    int min_py = -chunk_radius * cph;
    int max_py = chunk_radius * cph;

    std::uniform_int_distribution<int> dist_x(min_px, max_px - 1);
    std::uniform_int_distribution<int> dist_y(min_py, max_py - 1);

    int attempts = 0;
    constexpr int MAX_ATTEMPTS = 10000;

    while (static_cast<int>(pending_predetermined_positions.size()) < structure_spawn_count && attempts < MAX_ATTEMPTS)
    {
        attempts++;
        glm::ivec2 candidate(dist_x(rng), dist_y(rng));

        // Snap to PARTICLE_SIZE grid so structure cells map 1:1 to world cells
        int ps = static_cast<int>(Globals::PARTICLE_SIZE);
        candidate.x = (candidate.x / ps) * ps;
        candidate.y = (candidate.y / ps) * ps;

        // Enforce minimum distance between all predetermined positions
        bool too_close = false;
        for (const auto &existing : pending_predetermined_positions)
        {
            if (glm::distance(glm::vec2(candidate), glm::vec2(existing)) < MIN_PREDETERMINED_DISTANCE)
            {
                too_close = true;
                break;
            }
        }
        if (too_close)
            continue;

        pending_predetermined_positions.push_back(candidate);
    }

    // Keep a permanent copy for the UI
    predetermined_spawn_positions = pending_predetermined_positions;
}

void World::try_place_predetermined_structures(const glm::ivec2 &chunk_coords)
{
    auto struct_it = image_structures.find("devushki_column");
    if (struct_it == image_structures.end())
        return; // image not loaded

    const Structure &structure = struct_it->second;
    int cpw = static_cast<int>(chunk_width * Globals::PARTICLE_SIZE);
    int cph = static_cast<int>(chunk_height * Globals::PARTICLE_SIZE);

    // Structure pixel footprint
    int struct_pw = static_cast<int>(structure.get_width() * Globals::PARTICLE_SIZE);
    int struct_ph = static_cast<int>(structure.get_height() * Globals::PARTICLE_SIZE);

    auto it = pending_predetermined_positions.begin();
    while (it != pending_predetermined_positions.end())
    {
        const glm::ivec2 &pos = *it;

        // Check if the newly loaded chunk overlaps the structure's footprint at all
        int chunk_min_x = chunk_coords.x * cpw;
        int chunk_min_y = chunk_coords.y * cph;
        int chunk_max_x = chunk_min_x + cpw;
        int chunk_max_y = chunk_min_y + cph;

        // Structure occupies [pos.x, pos.x + struct_pw) x [pos.y, pos.y + struct_ph)
        bool overlaps = pos.x < chunk_max_x && (pos.x + struct_pw) > chunk_min_x &&
                        pos.y < chunk_max_y && (pos.y + struct_ph) > chunk_min_y;

        if (!overlaps)
        {
            ++it;
            continue;
        }

        // This chunk overlaps the structure. Check if ALL chunks the structure spans exist.
        bool all_chunks_exist = true;
        int first_chunk_x = static_cast<int>(std::floor(static_cast<float>(pos.x) / cpw));
        int first_chunk_y = static_cast<int>(std::floor(static_cast<float>(pos.y) / cph));
        int last_chunk_x = static_cast<int>(std::floor(static_cast<float>(pos.x + struct_pw - 1) / cpw));
        int last_chunk_y = static_cast<int>(std::floor(static_cast<float>(pos.y + struct_ph - 1) / cph));

        for (int cy = first_chunk_y; cy <= last_chunk_y && all_chunks_exist; cy++)
            for (int cx = first_chunk_x; cx <= last_chunk_x && all_chunks_exist; cx++)
                if (!get_chunk(glm::ivec2(cx, cy)))
                    all_chunks_exist = false;

        if (!all_chunks_exist)
        {
            ++it; // Not ready yet, try again when another chunk loads
            continue;
        }

        // Check if original position is valid
        if (check_placement_valid(structure, pos))
        {
            // Place at original position
            structure_spawner.place_structure(structure, pos);
            it = pending_predetermined_positions.erase(it);
            continue;
        }

        // Original position failed — search for a valid nearby spot
        int idx = static_cast<int>(std::distance(pending_predetermined_positions.begin(), it));
        glm::ivec2 new_pos = find_valid_nearby_position(structure, pos, idx);

        if (new_pos.x != INT_MIN)
        {
            // Found a valid alternative — place there
            structure_spawner.place_structure(structure, new_pos);

            // Update the permanent UI record to show actual placement location
            for (auto &ppos : predetermined_spawn_positions)
            {
                if (ppos == pos)
                {
                    ppos = new_pos;
                    break;
                }
            }
            it = pending_predetermined_positions.erase(it);
        }
        else
        {
            // No valid spot found anywhere nearby — discard permanently
            it = pending_predetermined_positions.erase(it);
        }
    }
}

bool World::check_placement_valid(const Structure &structure, const glm::ivec2 &pos)
{
    int cpw = static_cast<int>(chunk_width * Globals::PARTICLE_SIZE);
    int cph = static_cast<int>(chunk_height * Globals::PARTICLE_SIZE);
    int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    int struct_pw = static_cast<int>(structure.get_width() * Globals::PARTICLE_SIZE);
    int struct_ph = static_cast<int>(structure.get_height() * Globals::PARTICLE_SIZE);

    // Check all spanned chunks exist
    int first_cx = static_cast<int>(std::floor(static_cast<float>(pos.x) / cpw));
    int first_cy = static_cast<int>(std::floor(static_cast<float>(pos.y) / cph));
    int last_cx = static_cast<int>(std::floor(static_cast<float>(pos.x + struct_pw - 1) / cpw));
    int last_cy = static_cast<int>(std::floor(static_cast<float>(pos.y + struct_ph - 1) / cph));

    for (int cy = first_cy; cy <= last_cy; cy++)
        for (int cx = first_cx; cx <= last_cx; cx++)
            if (!get_chunk(glm::ivec2(cx, cy)))
                return false;

    // Check empty ratio >= 70%
    int total_cells = 0;
    int empty_cells = 0;

    for (int sy = 0; sy < structure.get_height(); sy++)
    {
        for (int sx = 0; sx < structure.get_width(); sx++)
        {
            total_cells++;
            glm::ivec2 pixel_pos = pos + glm::ivec2(sx * ps, sy * ps);

            glm::ivec2 cpos{
                static_cast<int>(std::floor(static_cast<float>(pixel_pos.x) / cpw)),
                static_cast<int>(std::floor(static_cast<float>(pixel_pos.y) / cph))};

            Chunk *c = get_chunk(cpos);
            if (!c)
                continue;

            int off_x = pixel_pos.x - cpos.x * cpw;
            int off_y = pixel_pos.y - cpos.y * cph;
            int cell_x = off_x / ps;
            int cell_y = off_y / ps;
            if (cell_x < 0)
                cell_x += chunk_width;
            if (cell_y < 0)
                cell_y += chunk_height;

            if (cell_x >= 0 && cell_x < chunk_width && cell_y >= 0 && cell_y < chunk_height)
            {
                if (c->is_empty(cell_x, cell_y))
                    empty_cells++;
            }
        }
    }

    float empty_ratio = (total_cells > 0) ? static_cast<float>(empty_cells) / total_cells : 0.0f;
    if (empty_ratio < 0.7f)
        return false;

    // Check solid ground beneath bottom edge (>= 30%)
    int ground_y_px = pos.y + struct_ph;
    int solid_ground_count = 0;
    int ground_samples = 0;

    for (int gx = pos.x; gx < pos.x + struct_pw; gx += ps)
    {
        ground_samples++;

        glm::ivec2 ground_pixel{gx, ground_y_px};
        glm::ivec2 gchunk_pos{
            static_cast<int>(std::floor(static_cast<float>(ground_pixel.x) / cpw)),
            static_cast<int>(std::floor(static_cast<float>(ground_pixel.y) / cph))};

        Chunk *gchunk = get_chunk(gchunk_pos);
        if (!gchunk)
            continue;

        int goff_x = ground_pixel.x - gchunk_pos.x * cpw;
        int goff_y = ground_pixel.y - gchunk_pos.y * cph;
        int gcell_x = goff_x / ps;
        int gcell_y = goff_y / ps;

        if (gcell_x < 0)
            gcell_x += chunk_width;
        if (gcell_y < 0)
            gcell_y += chunk_height;

        if (gcell_x >= 0 && gcell_x < chunk_width && gcell_y >= 0 && gcell_y < chunk_height)
        {
            if (!gchunk->is_empty(gcell_x, gcell_y))
                solid_ground_count++;
        }
    }

    if (ground_samples == 0 || static_cast<float>(solid_ground_count) / ground_samples < 0.3f)
        return false;

    return true;
}

glm::ivec2 World::find_valid_nearby_position(const Structure &structure, const glm::ivec2 &original_pos, int index)
{
    int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    int cpw = static_cast<int>(chunk_width * Globals::PARTICLE_SIZE);
    int cph = static_cast<int>(chunk_height * Globals::PARTICLE_SIZE);
    constexpr float MIN_PREDETERMINED_DISTANCE = 800.0f;
    constexpr int SEARCH_RADIUS = 500; // pixels
    constexpr int SEARCH_STEP = 50;    // pixels (one chunk width)

    // Use deterministic RNG seeded per-position so results are reproducible
    std::mt19937 rng(static_cast<unsigned>(1 + index * 7919));

    // Collect candidate offsets in expanding rings
    std::vector<glm::ivec2> candidates;
    for (int radius = SEARCH_STEP; radius <= SEARCH_RADIUS; radius += SEARCH_STEP)
    {
        for (int dx = -radius; dx <= radius; dx += SEARCH_STEP)
        {
            for (int dy = -radius; dy <= radius; dy += SEARCH_STEP)
            {
                // Only consider positions on the current ring's perimeter
                if (std::abs(dx) != radius && std::abs(dy) != radius)
                    continue;

                glm::ivec2 cand = original_pos + glm::ivec2(dx, dy);
                // Snap to grid
                cand.x = (cand.x / ps) * ps;
                cand.y = (cand.y / ps) * ps;

                candidates.push_back(cand);
            }
        }
    }

    // Shuffle for variety, but deterministically
    std::shuffle(candidates.begin(), candidates.end(), rng);

    // Sort by distance from original so we prefer closer alternatives
    std::stable_sort(candidates.begin(), candidates.end(),
                     [&original_pos](const glm::ivec2 &a, const glm::ivec2 &b)
                     {
                         float da = glm::distance(glm::vec2(a), glm::vec2(original_pos));
                         float db = glm::distance(glm::vec2(b), glm::vec2(original_pos));
                         return da < db;
                     });

    for (const auto &cand : candidates)
    {
        // Check minimum distance to all OTHER predetermined positions (placed and pending)
        bool too_close = false;
        for (const auto &existing : predetermined_spawn_positions)
        {
            if (existing == original_pos)
                continue; // skip self
            if (glm::distance(glm::vec2(cand), glm::vec2(existing)) < MIN_PREDETERMINED_DISTANCE)
            {
                too_close = true;
                break;
            }
        }
        if (too_close)
            continue;

        // Check the candidate is within loaded world area
        int min_px = -chunk_radius * cpw;
        int max_px = chunk_radius * cpw;
        int min_py = -chunk_radius * cph;
        int max_py = chunk_radius * cph;
        if (cand.x < min_px || cand.x >= max_px || cand.y < min_py || cand.y >= max_py)
            continue;

        // Check placement rules
        if (check_placement_valid(structure, cand))
            return cand;
    }

    return glm::ivec2(INT_MIN, INT_MIN); // no valid position found
}

const std::vector<glm::ivec2> &World::get_predetermined_positions() const
{
    return predetermined_spawn_positions;
}

const std::vector<glm::ivec2> &World::get_pending_predetermined_positions() const
{
    return pending_predetermined_positions;
}
