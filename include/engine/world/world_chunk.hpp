#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "engine/world/world_cell.hpp"

enum class Chunk_States
{
    UNLOADED, // non active
    LOADED    // active
};

/* 
 * Chunk
 * @param width - , 
 * @param height - , 
 * @param coords coords in the world
 */
class Chunk
{
private:
    std::vector<WorldCell> chunk_data;
    // std::vector<std::pair<int, int>> changes; // co sa zmenilo/pohlo z povodneho indexu na iny index

    Chunk_States state = Chunk_States::UNLOADED;

private:
    void init_chunk_data();
    inline int get_index(int x, int y);

    void move_cell();
    // void commit_cells();

public:
    const int width, height;
    const glm::ivec2 coords;

public:
    Chunk(glm::ivec2 coords, int width, int height);
    ~Chunk() = default;

    Chunk_States get_state();
    void set_state(Chunk_States state);

    bool is_empty(int x, int y);
    bool is_empty(int index);

    void set_worldcell(int x, int y, Particle* particle);
    void set_worldcell(int index, Particle* particle);
    WorldCell* get_worldcell(int x, int y);
    WorldCell* get_worldcell(int index);
};
