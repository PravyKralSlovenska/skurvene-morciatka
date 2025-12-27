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
    const int seed; // seed na generovanie nahodneho sveta

    Player *player;

    const int chunk_width = 50;
    const int chunk_height = 50;
    const int chunk_radius = 5; // kolko chunkov by sa malo aktualizovat/generovat okolo hraca

    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> world; // chunks
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> active_chunks;
    // std::unique_ptr<Herringbone_World_Generation> world_gen;
    std::unique_ptr<World_CA_Generation> world_gen;

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

    void place_particle(const glm::ivec2 position, const Particle_Type particle_type);

    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> *get_chunks();
    int get_chunks_size();
    std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> *get_active_chunks();

    std::pair<int, int> get_chunk_dimensions();
    Chunk *get_chunk(const int x, const int y);
    Chunk *get_chunk(const glm::ivec2 &coords);
    Chunk *get_chunk(const Chunk_Coords_to_Hash something);
};
