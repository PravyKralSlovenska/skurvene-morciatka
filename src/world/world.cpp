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
#include <cstdlib>

namespace
{
    int resolve_world_seed_from_env_or_random()
    {
        const char *seed_env = std::getenv("MORCIATKO_WORLD_SEED");
        if (seed_env && seed_env[0] != '\0')
        {
            char *end = nullptr;
            const long parsed = std::strtol(seed_env, &end, 10);
            if (end != seed_env)
            {
                return static_cast<int>(parsed);
            }
        }

        return static_cast<int>(std::random_device{}());
    }
}

World::World()
{
    world_seed = resolve_world_seed_from_env_or_random();

    world_gen = std::make_unique<World_CA_Generation>(chunk_width, chunk_height);
    world_gen->set_seed(world_seed);

    // Initialize falling sand simulation
    sand_simulation = std::make_unique<Falling_Sand_Simulation>();
    sand_simulation->set_world(this);

    // Initialize structure spawner
    structure_spawner.set_world(this);
    structure_spawner.set_seed(world_seed);

    // Load structure blueprints and generate deterministic targets.
    load_image_structures("../structure_images");
    structure_spawner.generate_predetermined_positions(world_seed);
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

World_CA_Generation *World::get_world_gen()
{
    return world_gen.get();
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

int World::get_seed() const
{
    return world_seed;
}

void World::regenerate_with_seed(int seed)
{
    world_seed = seed;
    world.clear();
    active_chunks.clear();

    if (world_gen)
    {
        world_gen->set_seed(world_seed);
    }

    structure_spawner.set_seed(world_seed);
    structure_spawner.generate_predetermined_positions(world_seed);
}

void World::regenerate_random_seed()
{
    regenerate_with_seed(static_cast<int>(std::random_device{}()));
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
        structure_spawner.try_place_pending_structures(coords);
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

void World::set_devushki_column_spawn_count(int count)
{
    structure_spawner.set_structure_spawn_count("devushki_column", count);
    structure_spawner.generate_predetermined_positions(world_seed);

    // Resolve column placements up-front so objective spawn positions are stable before gameplay starts.
    const int placement_passes = std::max(12, std::max(1, count) * 6);
    structure_spawner.place_pending_for_structure("devushki_column", placement_passes);
}

void World::set_devushki_column_spawn_radius_particles(int radius_particles)
{
    structure_spawner.set_devushki_spawn_radius_particles(radius_particles);
}

int World::get_devushki_column_spawn_radius_particles() const
{
    return structure_spawner.get_devushki_spawn_radius_particles();
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

    for (const auto &pair : image_structures)
    {
        if (pair.first == "devushki_store")
        {
            structure_spawner.add_blueprint("store", pair.second);
            continue;
        }

        structure_spawner.add_blueprint(pair.first, pair.second);
    }
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

std::vector<glm::ivec2> World::get_devushki_column_spawn_positions() const
{
    std::vector<glm::ivec2> spawn_positions;

    // Get the devushki_column structure for its dimensions
    auto struct_it = image_structures.find("devushki_column");
    if (struct_it == image_structures.end())
    {
        return spawn_positions;
    }

    const Structure &structure = struct_it->second;
    int struct_width_px = static_cast<int>(structure.get_width() * Globals::PARTICLE_SIZE);

    // Use placed structures from the spawner to get actual positions
    const auto &placed = structure_spawner.get_placed_structures();

    for (const auto &ps : placed)
    {
        if (ps.name == "devushki_column")
        {
            // Calculate center-top position of the column
            // pos is top-left corner, so center is at x + width/2, y stays at top
            glm::ivec2 center_pos;
            center_pos.x = ps.position.x + struct_width_px / 2;
            center_pos.y = ps.position.y; // top of the structure
            spawn_positions.push_back(center_pos);
        }
    }

    return spawn_positions;
}
