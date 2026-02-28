#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <random>
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
class World
{
private:
    Player *player;

    const int chunk_width = 10;
    const int chunk_height = 10;
    const int chunk_radius = 15; // kolko chunkov by sa malo aktualizovat/generovat okolo hraca

    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> world; // chunks
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> active_chunks;
    // std::unique_ptr<Herringbone_World_Generation> world_gen;
    std::unique_ptr<World_CA_Generation> world_gen;

    // Falling sand simulation
    std::unique_ptr<Falling_Sand_Simulation> sand_simulation;
    bool simulation_enabled = true;

    // Structure spawning
    StructureSpawner structure_spawner;

    // Image-loaded structures
    std::map<std::string, Structure> image_structures;

    // Predetermined structure spawn positions
    std::vector<glm::ivec2> predetermined_spawn_positions;   // permanent record (never erased)
    std::vector<glm::ivec2> pending_predetermined_positions; // working list (erased on place)
    int structure_spawn_count = 5;

    void generate_predetermined_positions();
    void try_place_predetermined_structures(const glm::ivec2 &chunk_coords);
    bool check_placement_valid(const Structure &structure, const glm::ivec2 &pos);
    glm::ivec2 find_valid_nearby_position(const Structure &structure, const glm::ivec2 &original_pos, int index);

private:
    inline int get_index(int x, int y);

    int change_width(int n);
    int change_height(int n);

    void calculate_active_chunks();
    void update_active_chunks();

    std::unique_ptr<Chunk> create_chunk(const int x, const int y);
    std::unique_ptr<Chunk> create_chunk(const glm::ivec2 &coords);

    void add_chunk(int x, int y);
    void add_chunk(glm::ivec2 coords);

    void remove_chunk(int x, int y);
    void remove_chunk(int index);

    // world generation
    void iterate(Chunk *chunk);
    std::vector<WorldCell *> find_solid_neighbors(WorldCell *cell, Chunk *chunk);

public:
    int width, height; // width and height are in chunks (world is 3 chunks long and 5 chunks high)

public:
    World();
    ~World();

    void set_player(Player *player);

    void update();
    void update(float delta_time); // Update with delta time for physics

    // Simulation control
    void enable_simulation(bool enabled);
    bool is_simulation_enabled() const;
    Falling_Sand_Simulation *get_simulation();

    // Place a non-static particle (for player interaction)
    void place_particle(const glm::ivec2 position, const Particle_Type particle_type);

    // Place a static particle (for terrain/world building)
    void place_static_particle(const glm::ivec2 position, const Particle_Type particle_type);

    // Place a fully constructed particle directly (for custom-colored particles from image structures)
    void place_custom_particle(const glm::ivec2 position, const Particle &particle);

    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> *get_chunks();
    int get_chunks_size();
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> *get_active_chunks();

    glm::ivec2 get_chunk_dimensions();
    Chunk *get_chunk(const int x, const int y);
    Chunk *get_chunk(const glm::ivec2 &coords);
    Chunk *get_chunk(const Chunk_Coords_to_Hash something);

    // pre hraca
    bool is_cell_in_hitbox(const Entity entity);

    // Structure spawning
    StructureSpawner &get_structure_spawner();
    void place_structure(const Structure &structure, const glm::ivec2 &world_pos);
    void place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos);

    // Image-loaded structures
    void load_image_structures(const std::string &folder_path);
    const std::map<std::string, Structure> &get_image_structures() const;
    Structure *get_image_structure(const std::string &name);

    // Predetermined structure spawn positions
    const std::vector<glm::ivec2> &get_predetermined_positions() const;
    const std::vector<glm::ivec2> &get_pending_predetermined_positions() const;
};
