#pragma once

#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"

#include "engine/world/FastNoiseLite.h"
#include "others/GLOBALS.hpp"

// forward declarations
class Chunk;
class Particle;
struct WorldCell;
struct Biome;
struct Biome_Blend;
enum class Biome_Type;
enum class Particle_Type;

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

    FastNoiseLite biome_noise;
    // FastNoiseLite details_noise;

    // const int NOISE_DENSITY = 47; // v percentach

    std::unordered_map<Biome_Type, FastNoiseLite> biome_cave_noises;

    std::vector<WorldCell> default_chunk_data;

private:
    inline glm::ivec2 calculate_world_coords(const glm::ivec2 &local_coords, const glm::ivec2 &chunk_coords, const glm::ivec2 &chunk_dimensions);
    inline float get_noise_value(const FastNoiseLite &noise, const glm::ivec2 &world_coords);

    // ked sa zmeni seed musi sa zmenit aj noise
    void recalculate_noises();
    void setup_biome_noises();

    Biome_Blend get_biome_blend(const glm::ivec2 &world_coords);
    float smoothstep(const float t, const float min = 0.0, const float max = 1.0);
    float linear_interpolation(const float a, const float b, const float t);

    Particle create_particle_by_type(const Particle_Type type);
    Particle_Type blend_particle_types(const Particle_Type primary_type, const Particle_Type secondary_type, const float blend_factor, const glm::ivec2 &world_coords);

public:
    World_CA_Generation(const int chunk_width, const int chunk_height);

    void set_seed(const int seed);

    // noise world generation
    void fill_chunk(Chunk *chunk);
    void generate_chunk_with_biome(Chunk *chunk);
    void carve_cave_noise(Chunk *chunk);
    void carve_cave_multinoise(Chunk *chunk);
    void carve_cave_iteration(Chunk *chunk);

    Biome get_biome(const glm::vec2 &world_coords);

    // cellural automata world generartion
    void make_noise_data(Chunk *chunk);
    void iteration(Chunk *chunk);
    void erosion(Chunk *chunk);
    void swap_datas();
};
