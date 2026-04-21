#pragma once

#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"

#include "engine/world/FastNoiseLite.h"
#include "engine/world/world_cell.hpp"
#include "others/GLOBALS.hpp"

// forward declarations
class Chunk;
class Particle;
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
    FastNoiseLite material_blend_noise;
    // FastNoiseLite details_noise;

    // const int NOISE_DENSITY = 47; // v percentach

    std::unordered_map<Biome_Type, FastNoiseLite> biome_cave_noises;

    std::vector<WorldCell> default_chunk_data;

private:
    // Calculates world coords.
    inline glm::ivec2 calculate_world_coords(const glm::ivec2 &local_coords, const glm::ivec2 &chunk_coords, const glm::ivec2 &chunk_dimensions);
    // Returns noise value.
    inline float get_noise_value(const FastNoiseLite &noise, const glm::ivec2 &world_coords);

    // ked sa zmeni seed musi sa zmenit aj noise
    void recalculate_noises();
    // Sets up biome noises.
    void setup_biome_noises();

    // Returns biome blend.
    Biome_Blend get_biome_blend(const glm::ivec2 &world_coords);
    // Smoothstep.
    float smoothstep(const float t, const float min = 0.0, const float max = 1.0);
    // Performs linear interpolation.
    float linear_interpolation(const float a, const float b, const float t);

    // Creates particle by type.
    Particle create_particle_by_type(const Particle_Type type);
    // Blends particle types.
    Particle_Type blend_particle_types(const Particle_Type primary_type, const Particle_Type secondary_type, const float blend_factor, const glm::ivec2 &world_coords);

public:
    // Constructs World_CA_Generation.
    World_CA_Generation(const int chunk_width, const int chunk_height);

    // Sets seed.
    void set_seed(const int seed);

    // noise world generation
    void fill_chunk(Chunk *chunk);
    // Generates chunk with biome.
    void generate_chunk_with_biome(Chunk *chunk);
    // Carves cave noise.
    void carve_cave_noise(Chunk *chunk);
    // Carves cave multinoise.
    void carve_cave_multinoise(Chunk *chunk);
    // Carves cave iteration.
    void carve_cave_iteration(Chunk *chunk);

    // Returns biome.
    Biome get_biome(const glm::vec2 &world_coords);

    // Query whether a cell at the given world cell coordinates would be solid,
    // using only noise (no Chunk needed).
    bool is_cell_solid(int world_cell_x, int world_cell_y);

    // cellural automata world generartion
    void make_noise_data(Chunk *chunk);
    // Runs iteration.
    void iteration(Chunk *chunk);
    // Applies erosion.
    void erosion(Chunk *chunk);
    // Swaps datas.
    void swap_datas();
};
