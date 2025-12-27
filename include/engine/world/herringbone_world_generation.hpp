#pragma once

#include "stb/stb_herringbone_wang_tile.h"

class Herringbone_World_Generation
{
private:
    const int seed;
    stbhw_tileset tileset;
    unsigned char *image_data_buffer;

public:
    Herringbone_World_Generation(const int seed);
    ~Herringbone_World_Generation();

    bool load_tileset_from_image(const char *path);
    bool generate_map(const char *output_filename, const int output_width, const int output_height);

    unsigned char *get_image_data();
};

// stbhw_tile genesis_tile = {};