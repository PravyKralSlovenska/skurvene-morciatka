#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

#include "engine/world/world_cell.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/herringbone_world_generation.hpp"
#include "engine/particle/particle.hpp"
#include "engine/entity.hpp"

/*
 * WORLD
 * - nig
 */
class World
{
private:
    int seed; // seed na generovanie nahodneho sveta
    // const Chunk GENESIS_CHUNK(glm::ivec2(0, 0));

    std::unique_ptr<Herringbone_World_Generation> world_gen;
    // std::unordered_map<glm::ivec2, Chunk*> world;
    std::vector<Chunk*> world;
    std::vector<Chunk*> active_chunks;

private: 
    int get_index(int x, int y);

    Chunk create_chunk();
    void add_chunk(Chunk chunk);
    void remove_chunk(int index);

public:
    int width, height; // width and height are in chunks (world is 3 chunks long and 5 chunks high)
    float scale;

public:
    World();
    ~World() = default;

    Chunk& get_chunk(int x, int y);
    Chunk& get_chunk(int index);
};
