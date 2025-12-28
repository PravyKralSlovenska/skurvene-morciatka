#pragma once

#include "others/GLOBALS.hpp"

// forward declarations
class Chunk;

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
    const int NOISE_DENSITY = 47; // v percentach
    // const int NOISE_DENSITY = Globals::FILL_PERCENT; // v percentach

public:
    void make_noise_data(Chunk *chunk);
    // kazdy biome by mohol mat x iteracii
    void iteration(Chunk *chunk);
    void erosion(Chunk *chunk);
    void swap_datas();
};
