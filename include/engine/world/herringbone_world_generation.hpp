#pragma once

#include <iostream>
#include <cstdlib>
#include <ctime>

#include "stb/stb_herringbone_wang_tile.h"
#include "others/utils.hpp"

class Herringbone_World_Generation
{
private:
    int seed;
    stbhw_tileset tileset;

public:
    Herringbone_World_Generation(int seed);
    ~Herringbone_World_Generation();

    bool load_tileset_from_image(const char *path);

    bool generate_map(const char *output_filename, int output_width, int output_height);
};
