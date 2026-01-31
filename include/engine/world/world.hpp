#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <glm/glm.hpp>

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
        return std::hash<int>()(coords.x) ^ (std::hash<int>()(coords.y) << 1);
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
    const int chunk_radius = 25; // kolko chunkov by sa malo aktualizovat/generovat okolo hraca

    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> world; // chunks
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> active_chunks;
    // std::unique_ptr<Herringbone_World_Generation> world_gen;
    std::unique_ptr<World_CA_Generation> world_gen;

    // Falling sand simulation
    std::unique_ptr<Falling_Sand_Simulation> sand_simulation;
    bool simulation_enabled = true;

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

    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> *get_chunks();
    int get_chunks_size();
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> *get_active_chunks();

    glm::ivec2 get_chunk_dimensions();
    Chunk *get_chunk(const int x, const int y);
    Chunk *get_chunk(const glm::ivec2 &coords);
    Chunk *get_chunk(const Chunk_Coords_to_Hash something);

    // pre hraca
    bool is_cell_in_hitbox(const Entity entity);
};
