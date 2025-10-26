#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "engine/world/world_cell.hpp"

enum class Chunk_States
{
    LOADED,  // active
    UNLOADED // non active
};

/* 
 *
 * @param width - , 
 * @param height - , 
 * @param coords coords in the world
 */
class Chunk
{
private:
    std::vector<WorldCell> chunk_data;
    // std::vector<std::pair<int, int>> changes;

    Chunk_States state = Chunk_States::UNLOADED;

private:
    void init_chunk_data();
    inline int get_index(int x, int y);

public:
    int width, height;
    glm::ivec2 coords;

public:
    Chunk(glm::ivec2 coords);
    Chunk(glm::ivec2 coords, int width, int height);
    ~Chunk();

    void set_width();
    void set_height();

    bool is_empty(int x, int y);
    bool is_empty(int index);
};

// teda nejaky hlavny chunk
