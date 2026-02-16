#pragma once

#include <vector>
#include <string>
#include <map>
#include <random>
#include <array>
#include <glm/glm.hpp>

#include "engine/particle/particle.hpp"

// forward declarations
class World;

// Structure blueprint - a 2D grid of Particles that can be stamped into the world.
// Empty particles (Particle_Type::EMPTY) are skipped during placement — they do NOT carve terrain.
class Structure
{
private:
    int width = 0; // in cells (each cell = 1 particle)
    int height = 0;
    std::vector<Particle> cells;
    std::string name;

public:
    Structure() = default;
    Structure(const std::string &name, int width, int height);

    // Build the structure cell by cell
    void set_cell(int x, int y, const Particle &particle);
    void set_cell(int x, int y, Particle_Type type, bool is_static = true);
    void fill_rect(int x, int y, int w, int h, Particle_Type type, bool is_static = true);

    // Getters
    int get_width() const { return width; }
    int get_height() const { return height; }
    const std::string &get_name() const { return name; }
    const Particle &get_cell(int x, int y) const;
    bool in_bounds(int x, int y) const;

    // Count non-empty cells
    int count_solid_cells() const;
};

// One default example structure
namespace StructureFactory
{
    Structure create_platform(int length = 8, Particle_Type material = Particle_Type::STONE);
};

// How a structure should be placed relative to terrain
enum class SpawnPlacement
{
    ON_SURFACE,   // Place so bottom sits on a solid surface with empty space above
    IN_OPEN_SPACE // Only place where the entire area is empty (air)
};

// Configuration for structure spawning during world generation
struct StructureSpawnRule
{
    std::string structure_name;       // Name of the structure blueprint to spawn
    float spawn_chance = 0.05f;       // Probability per chunk (0.0 - 1.0)
    float min_distance_same = 500.0f; // Min pixel distance from structures with same name
    float min_distance_any = 200.0f;  // Min pixel distance from ANY other structure
    SpawnPlacement placement = SpawnPlacement::ON_SURFACE;
    float min_empty_ratio = 0.7f; // At least this fraction of the area must be empty before placing
};

// Manages structure placement in the world
class StructureSpawner
{
private:
    World *world = nullptr;
    std::mt19937 rng;
    int seed = 0;

    // All available structure blueprints (by name)
    std::map<std::string, Structure> blueprints;

    // Track placed structures to enforce min distance
    struct PlacedStructure
    {
        glm::ivec2 position;
        std::string name;
    };
    std::vector<PlacedStructure> placed_structures;

    // Spawn rules
    std::vector<StructureSpawnRule> spawn_rules;

    // Pending structures
    struct PendingStructure
    {
        Structure structure;
        glm::ivec2 position;
        std::string name;
    };
    std::vector<PendingStructure> pending_structures;

private:
    bool check_min_distance(const glm::ivec2 &pos, const std::string &name,
                            float min_dist_same, float min_dist_any) const;
    float check_empty_ratio(const Structure &structure, const glm::ivec2 &world_pos) const;
    bool check_chunks_exist(const Structure &structure, const glm::ivec2 &world_pos) const;
    int find_surface_y(int world_x, const glm::ivec2 &chunk_world_origin, int chunk_pixel_height) const;

public:
    StructureSpawner();

    void set_world(World *world);
    void set_seed(int seed);

    // Manage blueprints
    void add_blueprint(const std::string &name, const Structure &structure);
    void add_blueprint(Structure &&structure);
    Structure *get_blueprint(const std::string &name);
    const std::map<std::string, Structure> &get_blueprints() const;

    // Spawn rules
    void add_spawn_rule(const StructureSpawnRule &rule);
    void clear_spawn_rules();
    void setup_default_rules();

    // Place a structure at a world position (top-left corner, in pixels).
    // Only places non-empty particles. Empty cells are skipped (terrain untouched).
    bool place_structure(const Structure &structure, const glm::ivec2 &world_pos);
    bool place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos);

    // Try to spawn structures in a chunk based on spawn rules
    void try_spawn_in_chunk(const glm::ivec2 &chunk_coords, int chunk_width, int chunk_height);
    void retry_pending_structures();

    const std::vector<PlacedStructure> &get_placed_structures() const;
};

// Loads structure blueprints from images in a folder.
// Transparent pixels become empty (skipped on placement).
// Known particle colors map to particle types; unknown colors become custom-colored stone.
// Images are auto-cropped to the bounding box of non-transparent content.
class ImageStructureLoader
{
public:
    static Structure load_from_image(const std::string &image_path);
    static std::map<std::string, Structure> load_all_from_folder(const std::string &folder_path);

private:
    static Particle pixel_to_particle(int r, int g, int b, int a);
    static bool is_pixel_empty(int r, int g, int b, int a);
    static float color_distance(int r1, int g1, int b1, int r2, int g2, int b2);
    static std::array<int, 4> find_content_bounds(const unsigned char *data, int width, int height, int channels);
};
