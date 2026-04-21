#pragma once

// File purpose: Defines structure blueprints, loading, and world placement systems.
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <random>
#include <array>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

#include "engine/particle/particle.hpp"

// forward declarations
class World;

// Structure blueprint - a 2D grid of Particles that can be stamped into the world.
// Empty particles (Particle_Type::EMPTY) are skipped during placement — they do NOT carve terrain.
// Represents a structure blueprint and placement helpers.
class Structure
{
private:
    int width = 0; // in cells (each cell = 1 particle)
    int height = 0;
    std::vector<Particle> cells;
    std::string name;

public:
    // Constructs Structure.
    Structure() = default;
    // Constructs Structure.
    Structure(const std::string &name, int width, int height);

    // Build the structure cell by cell
    void set_cell(int x, int y, const Particle &particle);
    // Sets cell.
    void set_cell(int x, int y, Particle_Type type, bool is_static = true);
    // Fills rect.
    void fill_rect(int x, int y, int w, int h, Particle_Type type, bool is_static = true);

    // Getters
    int get_width() const { return width; }
    // Returns height.
    int get_height() const { return height; }
    // Returns name.
    const std::string &get_name() const { return name; }
    // Returns cell.
    const Particle &get_cell(int x, int y) const;
    // In bounds.
    bool in_bounds(int x, int y) const;

    // Count non-empty cells
    int count_solid_cells() const;
};

// One default example structure
namespace StructureFactory
{
    // Creates platform.
    Structure create_platform(int length = 8, Particle_Type material = Particle_Type::STONE);
};

// Manages structure placement in the world
class StructureSpawner
{
public:
    // Defines the RuntimeStats struct.
    struct RuntimeStats
    {
        int calls = 0;
        double last_call_ms = 0.0;
        double avg_call_ms = 0.0;
        double max_call_ms = 0.0;
        int scanned_entries_last = 0;
        double scanned_entries_avg = 0.0;
        int placed_entries_last = 0;
        int added_store_targets_last = 0;
        int generated_chunks_last = 0;
        int desired_store_targets_last = 0;
        int current_store_targets_last = 0;
        int pending_entries_last = 0;
        int total_predetermined_entries_last = 0;
    };

    // Defines the PlacementProfile struct.
    struct PlacementProfile
    {
        int min_supported_columns = 1;
        float min_support_ratio = 0.20f;
        int support_scan_max_depth_cells = 18;
        int max_ground_variation_cells = 12;
    };

    // Defines the StoreSpawnConfig struct.
    struct StoreSpawnConfig
    {
        bool enabled = true;
        int chunks_per_spawn = 220;
        int min_generated_chunks_before_first_store = 120;
        int min_spawn_radius_particles = 600;
        int max_spawn_radius_particles = 2200;
        int min_distance_between_stores_particles = 800;
        int min_distance_from_columns_particles = 700;
        int min_distance_from_origin_particles = 500;

        // Deterministic region-grid placement (Minecraft-style random spread).
        // spacing_chunks/separation_chunks <= 0 means auto-derived from chunks_per_spawn.
        int spacing_chunks = 0;
        int separation_chunks = 0;
        int salt = 10387313;

        // Runtime work bounds.
        int active_region_radius_chunks = 24;
        int max_regions_processed_per_call = 1;
        int max_attempts_per_region = 3;
        int retry_generated_chunks = 384;
    };

    // Defines the PredeterminedEntry struct.
    struct PredeterminedEntry
    {
        std::string structure_name;
        glm::ivec2 target_pos;
        bool placed = false;
    };

    // Defines the StoreRegionHash struct.
    struct StoreRegionHash
    {
        std::size_t operator()(const glm::ivec2 &coords) const
        {
            std::size_t hash = 2166136261u;
            hash = (hash ^ static_cast<std::size_t>(coords.x)) * 16777619u;
            hash = (hash ^ static_cast<std::size_t>(coords.y)) * 16777619u;
            return hash;
        }
    };

    // Defines the StoreRegionData struct.
    struct StoreRegionData
    {
        glm::ivec2 candidate_pos{0, 0};
        glm::ivec2 candidate_chunk{0, 0};
        int attempts = 0;
        int retry_after_generated_chunks = 0;
        bool candidate_ready = false;
        bool placed = false;
        bool exhausted = false;
    };

private:
    World *world = nullptr;
    std::mt19937 rng;
    int seed = 0;
    int devushki_spawn_radius_particles = 500;
    StoreSpawnConfig store_spawn_config;
    PlacementProfile default_placement_profile;
    std::map<std::string, PlacementProfile> structure_placement_profiles;
    int store_target_sequence_index = 0;

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

    // Store placement state uses deterministic region coordinates instead of
    // a globally growing pending-entry list.
    std::unordered_map<glm::ivec2, StoreRegionData, StoreRegionHash> store_regions;
    std::deque<glm::ivec2> store_region_queue;
    std::unordered_set<glm::ivec2, StoreRegionHash> store_region_queued;

    RuntimeStats runtime_stats;
    int runtime_stats_log_tick = 0;

private:
    // Finds surface y.
    int find_surface_y(int world_x, int start_y, int scan_range) const;
    // Returns true if store name.
    bool is_store_name(const std::string &name) const;
    // Returns true if column name.
    bool is_column_name(const std::string &name) const;
    // Returns true if store target far enough.
    bool is_store_target_far_enough(const glm::ivec2 &target_px) const;
    // Tries generate store target for sequence.
    bool try_generate_store_target_for_sequence(int sequence_index, glm::ivec2 &out_target_px) const;
    // Sanitize placement profile.
    PlacementProfile sanitize_placement_profile(const PlacementProfile &profile) const;

public:
    // Structure Spawner.
    StructureSpawner();

    // Sets world.
    void set_world(World *world);
    // Sets seed.
    void set_seed(int seed);
    // Sets structure spawn count.
    void set_structure_spawn_count(const std::string &name, int count);
    // Sets devushki spawn radius particles.
    void set_devushki_spawn_radius_particles(int radius_particles);
    // Returns devushki spawn radius particles.
    int get_devushki_spawn_radius_particles() const;
    // Sets default placement profile.
    void set_default_placement_profile(const PlacementProfile &profile);
    // Returns default placement profile.
    const PlacementProfile &get_default_placement_profile() const;
    // Sets structure placement profile.
    void set_structure_placement_profile(const std::string &name, const PlacementProfile &profile);
    // Returns structure placement profile.
    bool get_structure_placement_profile(const std::string &name, PlacementProfile &out_profile) const;
    // Clears structure placement profile.
    void clear_structure_placement_profile(const std::string &name);
    // Resolves structure placement profile.
    PlacementProfile resolve_structure_placement_profile(const std::string &name) const;
    // Sets store spawn config.
    void set_store_spawn_config(const StoreSpawnConfig &config);
    // Returns store spawn config.
    const StoreSpawnConfig &get_store_spawn_config() const;
    // Sets store spawning enabled.
    void set_store_spawning_enabled(bool enabled);
    // Returns true if store spawning enabled.
    bool is_store_spawning_enabled() const;

    // Manage blueprints
    void add_blueprint(const std::string &name, const Structure &structure);
    // Adds blueprint.
    void add_blueprint(Structure &&structure);
    // Returns blueprint.
    Structure *get_blueprint(const std::string &name);
    // Returns blueprints.
    const std::map<std::string, Structure> &get_blueprints() const;

    // Place a structure at a world position (top-left corner, in pixels).
    // Only places non-empty particles. Empty cells are skipped (terrain untouched).
    bool place_structure(const Structure &structure, const glm::ivec2 &world_pos);
    // Place structure centered.
    bool place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos);
    // Fills structure base.
    void fill_structure_base(const Structure &structure, const glm::ivec2 &world_pos);

    // New deterministic, chunk-event-driven spawn workflow.
    void generate_predetermined_positions(int world_seed);
    // Tries place pending structures.
    void try_place_pending_structures(const glm::ivec2 &chunk_coords, int max_entries_to_place = -1);
    // Place pending for structure.
    void place_pending_for_structure(const std::string &name, int max_passes = 1);
    void prepare_pending_targets_for_structure(const std::string &name,
                                               int max_passes = 1,
                                               // No-op callback.
                                               const std::function<void(const std::string &, float)> &progress_callback = {});
    // Returns predetermined entries.
    const std::vector<PredeterminedEntry> &get_predetermined_entries() const;

    // Record a placed structure (for external placement tracking)
    void record_placed_structure(const glm::ivec2 &position, const std::string &name);

    // Returns placed structures.
    const std::vector<PlacedStructure> &get_placed_structures() const;
    // Returns runtime stats.
    RuntimeStats get_runtime_stats() const;
};

// Transparent pixels become empty (skipped on placement).
// Known particle colors map to particle types; unknown colors become custom-colored stone.
// Images are auto-cropped to the bounding box of non-transparent content.
// Loads structure blueprints from images.
class ImageStructureLoader
{
public:
    // Loads from image.
    static Structure load_from_image(const std::string &image_path);
    // Loads all from folder.
    static std::map<std::string, Structure> load_all_from_folder(const std::string &folder_path);

private:
    // Converts pixel to particle.
    static Particle pixel_to_particle(int r, int g, int b, int a);
    // Returns true if pixel empty.
    static bool is_pixel_empty(int r, int g, int b, int a);
    // Computes color distance.
    static float color_distance(int r1, int g1, int b1, int r2, int g2, int b2);
    // Finds content bounds.
    static std::array<int, 4> find_content_bounds(const unsigned char *data, int width, int height, int channels);
};
