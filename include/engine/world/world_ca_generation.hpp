#pragma once

#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"

#include "engine/world/FastNoiseLite.h"
#include "others/GLOBALS.hpp"

// forward declarations
class Chunk;
struct WorldCell;
class Biome;
enum class Biome_Type;

/*
 * World_CA_Generation
 * FAZY:
 *  1. BIOME MAP
 *  2. TEREN
 *  3. ZMEN TEREN PODLA BIOME
 *  4. STRUKTURY
 *  5. DEKORACIE
 */
class World_CA_Generation
{
private:
    int seed = 1234;

    FastNoiseLite cave_noise;
    FastNoiseLite biome_noise;
    // const int NOISE_DENSITY = 47; // v percentach

    std::unordered_map<Biome_Type, FastNoiseLite> biome_cave_noises;

    std::vector<WorldCell> default_chunk_data;

private:
    glm::ivec2 calculate_world_coords();

    // ked sa zmeni seed musi sa zmenit aj noise
    void recalculate_noises();
    void setup_biome_noises(); // nefunguje

public:
    World_CA_Generation(const int chunk_width, const int chunk_height);

    void set_seed(const int seed);

    // noise world generation
    void fill_chunk(Chunk *chunk);
    void generate_chunk_with_biome(Chunk *chunk);
    void carve_cave_noise(Chunk *chunk);
    void carve_cave_multinoise(Chunk *chunk);
    void carve_cave_iteration(Chunk *chunk);
    Biome get_biome(const int world_x, const int world_y);

    // cellural automata world generartion
    void make_noise_data(Chunk *chunk);
    void iteration(Chunk *chunk);
    void erosion(Chunk *chunk);
    void swap_datas();
};
