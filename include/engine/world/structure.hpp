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

// Manages structure placement in the world
class StructureSpawner
{
public:
    struct PredeterminedEntry
    {
        std::string structure_name;
        glm::ivec2 target_pos;
        bool placed = false;
    };

private:
    World *world = nullptr;
    std::mt19937 rng;
    int seed = 0;
    int devushki_spawn_radius_particles = 5000;

    // Optional per-structure override for deterministic predetermined entry count.
    std::map<std::string, int> structure_spawn_counts;

    // All available structure blueprints (by name)
    std::map<std::string, Structure> blueprints;

    // Track placed structures to enforce min distance
    struct PlacedStructure
    {
        glm::ivec2 position;
        std::string name;
    };
    std::vector<PlacedStructure> placed_structures;

    // One deterministic target per registered structure.
    std::vector<PredeterminedEntry> predetermined_entries;

private:
    int find_surface_y(int world_x, int start_y, int scan_range) const;

public:
    StructureSpawner();

    void set_world(World *world);
    void set_seed(int seed);
    void set_structure_spawn_count(const std::string &name, int count);
    void set_devushki_spawn_radius_particles(int radius_particles);
    int get_devushki_spawn_radius_particles() const;

    // Manage blueprints
    void add_blueprint(const std::string &name, const Structure &structure);
    void add_blueprint(Structure &&structure);
    Structure *get_blueprint(const std::string &name);
    const std::map<std::string, Structure> &get_blueprints() const;

    // Place a structure at a world position (top-left corner, in pixels).
    // Only places non-empty particles. Empty cells are skipped (terrain untouched).
    bool place_structure(const Structure &structure, const glm::ivec2 &world_pos);
    bool place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos);
    void fill_structure_base(const Structure &structure, const glm::ivec2 &world_pos);

    // New deterministic, chunk-event-driven spawn workflow.
    void generate_predetermined_positions(int world_seed);
    void try_place_pending_structures(const glm::ivec2 &chunk_coords);
    void place_pending_for_structure(const std::string &name, int max_passes = 1);
    const std::vector<PredeterminedEntry> &get_predetermined_entries() const;

    // Record a placed structure (for external placement tracking)
    void record_placed_structure(const glm::ivec2 &position, const std::string &name);

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
