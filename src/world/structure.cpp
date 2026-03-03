#include "engine/world/structure.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "others/GLOBALS.hpp"

#include "stb/stb_image.h"

#include <filesystem>
#include <iostream>
#include <cmath>
#include <algorithm>

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
    case Particle_Type::URANIUM:
        p = create_uranium(is_static);
        break;
    case Particle_Type::STONE:
        p = create_stone(is_static);
        break;
    case Particle_Type::WOOD:
        p = create_wood(is_static);
        break;
    case Particle_Type::FIRE:
        p = create_fire(is_static);
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

void StructureSpawner::add_spawn_rule(const StructureSpawnRule &rule)
{
    spawn_rules.push_back(rule);
}

void StructureSpawner::clear_spawn_rules()
{
    spawn_rules.clear();
}

void StructureSpawner::setup_default_rules()
{
    // Add the default platform blueprint
    add_blueprint("platform", StructureFactory::create_platform());

    // Add spawn rule for platforms
    StructureSpawnRule rule;
    rule.structure_name = "platform";
    rule.spawn_chance = 0.03f;
    rule.min_distance_any = 300.0f;
    rule.placement = SpawnPlacement::ON_SURFACE;
    add_spawn_rule(rule);
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
    return true;
}

bool StructureSpawner::place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos)
{
    glm::ivec2 offset(
        static_cast<int>((structure.get_width() / 2) * Globals::PARTICLE_SIZE),
        static_cast<int>((structure.get_height() / 2) * Globals::PARTICLE_SIZE));
    return place_structure(structure, center_pos - offset);
}

bool StructureSpawner::check_min_distance(const glm::ivec2 &pos, const std::string &name,
                                          float min_dist_same, float min_dist_any) const
{
    for (const auto &placed : placed_structures)
    {
        float dist = glm::distance(glm::vec2(pos), glm::vec2(placed.position));
        if (placed.name == name && dist < min_dist_same)
            return false;
        if (dist < min_dist_any)
            return false;
    }
    return true;
}

float StructureSpawner::check_empty_ratio(const Structure &structure, const glm::ivec2 &world_pos) const
{
    if (!world)
        return 0.0f;

    int total = 0;
    int empty = 0;

    int chunk_pixel_width = static_cast<int>(world->get_chunk_dimensions().x * Globals::PARTICLE_SIZE);
    int chunk_pixel_height = static_cast<int>(world->get_chunk_dimensions().y * Globals::PARTICLE_SIZE);

    for (int y = 0; y < structure.get_height(); y++)
    {
        for (int x = 0; x < structure.get_width(); x++)
        {
            total++;

            glm::ivec2 pixel_pos = world_pos + glm::ivec2(
                                                   static_cast<int>(x * Globals::PARTICLE_SIZE),
                                                   static_cast<int>(y * Globals::PARTICLE_SIZE));

            glm::ivec2 chunk_pos{
                static_cast<int>(std::floor(static_cast<float>(pixel_pos.x) / chunk_pixel_width)),
                static_cast<int>(std::floor(static_cast<float>(pixel_pos.y) / chunk_pixel_height))};

            Chunk *chunk = world->get_chunk(chunk_pos);
            if (!chunk)
            {
                // Non-existent chunk counts as non-empty
                continue;
            }

            int pixel_offset_x = pixel_pos.x - (chunk_pos.x * chunk_pixel_width);
            int pixel_offset_y = pixel_pos.y - (chunk_pos.y * chunk_pixel_height);

            int cell_x = static_cast<int>(pixel_offset_x / Globals::PARTICLE_SIZE);
            int cell_y = static_cast<int>(pixel_offset_y / Globals::PARTICLE_SIZE);

            if (cell_x < 0)
                cell_x += world->get_chunk_dimensions().x;
            if (cell_y < 0)
                cell_y += world->get_chunk_dimensions().y;

            if (chunk->is_empty(cell_x, cell_y))
                empty++;
        }
    }

    if (total == 0)
        return 0.0f;
    return static_cast<float>(empty) / static_cast<float>(total);
}

bool StructureSpawner::check_chunks_exist(const Structure &structure, const glm::ivec2 &world_pos) const
{
    if (!world)
        return false;

    int chunk_pixel_width = static_cast<int>(world->get_chunk_dimensions().x * Globals::PARTICLE_SIZE);
    int chunk_pixel_height = static_cast<int>(world->get_chunk_dimensions().y * Globals::PARTICLE_SIZE);

    for (int y = 0; y < structure.get_height(); y++)
    {
        for (int x = 0; x < structure.get_width(); x++)
        {
            glm::ivec2 pixel_pos = world_pos + glm::ivec2(
                                                   static_cast<int>(x * Globals::PARTICLE_SIZE),
                                                   static_cast<int>(y * Globals::PARTICLE_SIZE));

            glm::ivec2 chunk_pos{
                static_cast<int>(std::floor(static_cast<float>(pixel_pos.x) / chunk_pixel_width)),
                static_cast<int>(std::floor(static_cast<float>(pixel_pos.y) / chunk_pixel_height))};

            if (!world->get_chunk(chunk_pos))
                return false;
        }
    }
    return true;
}

int StructureSpawner::find_surface_y(int world_x, const glm::ivec2 &chunk_world_origin,
                                     int chunk_pixel_height) const
{
    if (!world)
        return -1;

    int chunk_pixel_width = static_cast<int>(world->get_chunk_dimensions().x * Globals::PARTICLE_SIZE);
    int cw = world->get_chunk_dimensions().x;
    int ch = world->get_chunk_dimensions().y;

    // Scan downward from the top of the chunk column
    for (int py = chunk_world_origin.y; py < chunk_world_origin.y + chunk_pixel_height; py += static_cast<int>(Globals::PARTICLE_SIZE))
    {
        glm::ivec2 pixel_pos{world_x, py};

        glm::ivec2 chunk_pos{
            static_cast<int>(std::floor(static_cast<float>(pixel_pos.x) / chunk_pixel_width)),
            static_cast<int>(std::floor(static_cast<float>(pixel_pos.y) / static_cast<float>(ch * Globals::PARTICLE_SIZE)))};

        Chunk *chunk = world->get_chunk(chunk_pos);
        if (!chunk)
            continue;

        int pixel_offset_x = pixel_pos.x - static_cast<int>(chunk_pos.x * chunk_pixel_width);
        int pixel_offset_y = pixel_pos.y - static_cast<int>(chunk_pos.y * ch * Globals::PARTICLE_SIZE);

        int cell_x = static_cast<int>(pixel_offset_x / Globals::PARTICLE_SIZE);
        int cell_y = static_cast<int>(pixel_offset_y / Globals::PARTICLE_SIZE);

        if (cell_x < 0)
            cell_x += cw;
        if (cell_y < 0)
            cell_y += ch;

        if (cell_x >= 0 && cell_x < cw && cell_y >= 0 && cell_y < ch)
        {
            if (!chunk->is_empty(cell_x, cell_y))
                return py;
        }
    }
    return -1; // No surface found
}

void StructureSpawner::try_spawn_in_chunk(const glm::ivec2 &chunk_coords, int chunk_width, int chunk_height)
{
    if (!world)
        return;

    int chunk_pixel_width = static_cast<int>(chunk_width * Globals::PARTICLE_SIZE);
    int chunk_pixel_height = static_cast<int>(chunk_height * Globals::PARTICLE_SIZE);

    glm::ivec2 chunk_world_origin{
        chunk_coords.x * chunk_pixel_width,
        chunk_coords.y * chunk_pixel_height};

    // Use deterministic RNG based on chunk coordinates and seed
    std::mt19937 chunk_rng(hash_coords(chunk_coords.x, chunk_coords.y, seed));
    std::uniform_real_distribution<float> chance_dist(0.0f, 1.0f);

    for (const auto &rule : spawn_rules)
    {
        // Roll spawn chance
        if (chance_dist(chunk_rng) > rule.spawn_chance)
            continue;

        Structure *blueprint = get_blueprint(rule.structure_name);
        if (!blueprint)
            continue;

        // Pick a random X within the chunk
        std::uniform_int_distribution<int> x_dist(0, chunk_pixel_width - 1);
        int spawn_x = chunk_world_origin.x + x_dist(chunk_rng);

        glm::ivec2 spawn_pos;

        if (rule.placement == SpawnPlacement::ON_SURFACE)
        {
            int surface_y = find_surface_y(spawn_x, chunk_world_origin, chunk_pixel_height);
            if (surface_y < 0)
                continue;

            // Place structure so its bottom sits on the surface
            spawn_pos = glm::ivec2(spawn_x, surface_y - static_cast<int>(blueprint->get_height() * Globals::PARTICLE_SIZE));
        }
        else // IN_OPEN_SPACE
        {
            std::uniform_int_distribution<int> y_dist(0, chunk_pixel_height - 1);
            spawn_pos = glm::ivec2(spawn_x, chunk_world_origin.y + y_dist(chunk_rng));
        }

        // Check minimum distance
        if (!check_min_distance(spawn_pos, rule.structure_name, rule.min_distance_same, rule.min_distance_any))
            continue;

        // Check that all required chunks exist
        if (!check_chunks_exist(*blueprint, spawn_pos))
        {
            // Queue as pending
            pending_structures.push_back({*blueprint, spawn_pos, rule.structure_name});
            continue;
        }

        // Check empty ratio
        if (check_empty_ratio(*blueprint, spawn_pos) < rule.min_empty_ratio)
            continue;

        // Place the structure
        place_structure(*blueprint, spawn_pos);
        placed_structures.push_back({spawn_pos, rule.structure_name});
    }
}

void StructureSpawner::retry_pending_structures()
{
    if (!world)
        return;

    auto it = pending_structures.begin();
    while (it != pending_structures.end())
    {
        if (check_chunks_exist(it->structure, it->position))
        {
            place_structure(it->structure, it->position);
            placed_structures.push_back({it->position, it->name});
            it = pending_structures.erase(it);
        }
        else
        {
            ++it;
        }
    }
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
