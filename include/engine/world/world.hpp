#pragma once

// File purpose: Defines world state, chunk storage, and world-level operations.
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <random>
#include <functional>
#include <glm/glm.hpp>

#include "engine/world/structure.hpp"
#include "engine/particle/particle.hpp"

// forward declarations
class Player;
class World_CA_Generation;
class Chunk;
class WorldCell;
class Entity;
class Falling_Sand_Simulation;
enum class Particle_Type;

/*
 * Chunk Coords to Hash
 * -
 */
struct Chunk_Coords_to_Hash
{
    std::size_t operator()(const glm::ivec2 &coords) const
    {
        size_t hash = 2166136261u;
        hash = (hash ^ static_cast<size_t>(coords.x)) * 16777619u;
        hash = (hash ^ static_cast<size_t>(coords.y)) * 16777619u;
        return hash;
    }
};

/*
 * WORLD
 * -
 */
// Owns chunks, world generation, and structure integration.
class World
{
private:
    Player *player;
    int world_seed = 0;

    const int chunk_width = 10;
    const int chunk_height = 10;
    const int chunk_radius = 15; // kolko chunkov by sa malo aktualizovat/generovat okolo hraca
    std::vector<glm::ivec2> cached_active_chunk_offsets;

    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> world; // chunks
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> active_chunks;
    // std::unique_ptr<Herringbone_World_Generation> world_gen;
    std::unique_ptr<World_CA_Generation> world_gen;

    // Falling sand simulation
    std::unique_ptr<Falling_Sand_Simulation> sand_simulation;
    bool simulation_enabled = true;

    // Structure spawning
    StructureSpawner structure_spawner;
    std::deque<glm::ivec2> pending_structure_chunk_events;
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> pending_structure_chunk_event_set;
    static constexpr int STRUCTURE_CHUNK_EVENTS_PER_FRAME_BUDGET = 1;

    // Image-loaded structures
    std::map<std::string, Structure> image_structures;

private:
    // Returns index.
    inline int get_index(int x, int y);

    // Changes width.
    int change_width(int n);
    // Changes height.
    int change_height(int n);

    // Calculates active chunks.
    void calculate_active_chunks();
    // Updates active chunks.
    void update_active_chunks();

    // Creates chunk.
    std::unique_ptr<Chunk> create_chunk(const int x, const int y);
    // Creates chunk.
    std::unique_ptr<Chunk> create_chunk(const glm::ivec2 &coords);

    // Adds chunk.
    void add_chunk(int x, int y);
    // Adds chunk.
    void add_chunk(glm::ivec2 coords);
    // Enqueue structure chunk event.
    void enqueue_structure_chunk_event(const glm::ivec2 &coords);
    // Processes structure chunk events.
    void process_structure_chunk_events();

    // Removes chunk.
    void remove_chunk(int x, int y);
    // Removes chunk.
    void remove_chunk(int index);

    // world generation
    void iterate(Chunk *chunk);
    // Finds solid neighbors.
    std::vector<WorldCell *> find_solid_neighbors(WorldCell *cell, Chunk *chunk);

public:
    int width, height; // width and height are in chunks (world is 3 chunks long and 5 chunks high)

public:
    // Constructs World.
    World();
    // Destroys World and releases owned resources.
    ~World();

    // Sets player.
    void set_player(Player *player);
    // Returns seed.
    int get_seed() const;
    // Regenerates with seed.
    void regenerate_with_seed(int seed);
    // Regenerates random seed.
    void regenerate_random_seed();

    // Updates state.
    void update();
    // Updates state.
    void update(float delta_time); // Update with delta time for physics

    // Simulation control
    void enable_simulation(bool enabled);
    // Returns true if simulation enabled.
    bool is_simulation_enabled() const;
    // Returns simulation.
    Falling_Sand_Simulation *get_simulation();
    // Returns world gen.
    World_CA_Generation *get_world_gen();

    // Place a non-static particle (for player interaction)
    void place_particle(const glm::ivec2 position, const Particle_Type particle_type);

    // Place a static particle (for terrain/world building)
    void place_static_particle(const glm::ivec2 position, const Particle_Type particle_type);

    // Place a fully constructed particle directly (for custom-colored particles from image structures)
    void place_custom_particle(const glm::ivec2 position, const Particle &particle);

    // Returns chunks.
    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> *get_chunks();
    // Returns chunks size.
    int get_chunks_size();
    // Returns active chunks.
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> *get_active_chunks();

    // Returns chunk dimensions.
    glm::ivec2 get_chunk_dimensions();
    // Returns chunk.
    Chunk *get_chunk(const int x, const int y);
    // Returns chunk.
    Chunk *get_chunk(const glm::ivec2 &coords);
    // Returns chunk.
    Chunk *get_chunk(const Chunk_Coords_to_Hash something);

    // pre hraca
    bool is_cell_in_hitbox(const Entity entity);

    // Structure spawning
    StructureSpawner &get_structure_spawner();
    void set_devushki_column_spawn_count(int count,
                                         // No-op callback.
                                         const std::function<void(const std::string &, float)> &progress_callback = {});
    // Sets devushki column spawn radius particles.
    void set_devushki_column_spawn_radius_particles(int radius_particles);
    // Returns devushki column spawn radius particles.
    int get_devushki_column_spawn_radius_particles() const;
    // Sets store spawn config.
    void set_store_spawn_config(const StructureSpawner::StoreSpawnConfig &config);
    // Returns store spawn config.
    const StructureSpawner::StoreSpawnConfig &get_store_spawn_config() const;
    // Sets store spawning enabled.
    void set_store_spawning_enabled(bool enabled);
    // Returns true if store spawning enabled.
    bool is_store_spawning_enabled() const;
    // Place structure.
    void place_structure(const Structure &structure, const glm::ivec2 &world_pos);
    // Place structure centered.
    void place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos);

    // Image-loaded structures
    void load_image_structures(const std::string &folder_path);
    // Returns image structures.
    const std::map<std::string, Structure> &get_image_structures() const;
    // Returns image structure.
    Structure *get_image_structure(const std::string &name);

    // Get devushki column center positions (for entity spawning)
    std::vector<glm::ivec2> get_devushki_column_spawn_positions() const;
};
