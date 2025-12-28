#pragma once

#include <vector>
#include <glm/glm.hpp>

// forward declarations
struct WorldCell;
class Vertex;
enum class Particle_Type;

enum class Chunk_States
{
    UNLOADED, // non active
    LOADED,   // active
    FINALL_FORM
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

    // pre renderer
    // bool is_dirty = true;
    std::vector<Vertex> cached_verticies;
    std::vector<unsigned int> cached_indicies;

private:
    void init_chunk_data();
    inline int get_index(int x, int y);

    void move_worldcell(WorldCell &from, WorldCell &to);
    // void commit_cells();

    void make_cached_verticies();
    void make_cached_indicies();

public:
    const int width, height;
    const glm::ivec2 coords;

public:
    Chunk(glm::ivec2 coords, int width, int height);
    ~Chunk() = default;

    void update(); // nieco

    std::vector<WorldCell> *get_chunk_data();
    void set_chunk_data(std::vector<WorldCell> &chunk_data);

    Chunk_States get_state();
    void set_state(Chunk_States state);

    bool is_dirty();

    bool is_empty(int x, int y);
    bool is_empty(int index);
    // vrati worldcell ak worldcell particle je typu empty
    WorldCell *get_if_not_empty(const int x, const int y);
    // vrati worldcell ak worldcell nie je prazdny
    WorldCell *get_if_not_empty(const int index);

    void set_worldcell(const glm::ivec2 &coords, Particle_Type particle);
    void set_worldcell(int x, int y, Particle_Type particle);
    void set_worldcell(int index, Particle_Type particle);
    WorldCell *get_worldcell(int x, int y);
    WorldCell *get_worldcell(int index);

    // get render data
    std::vector<Vertex> *get_cached_verticies();
    std::vector<unsigned int> *get_cached_indicies();
};
