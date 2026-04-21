#pragma once

// File purpose: Generates herringbone-style world patterns for testing.
#include "stb/stb_herringbone_wang_tile.h"

// Generates herringbone terrain patterns.
class Herringbone_World_Generation
{
private:
    const int seed;
    stbhw_tileset tileset;
    unsigned char *image_data_buffer;

public:
    // Constructs Herringbone_World_Generation.
    Herringbone_World_Generation(const int seed);
    // Destroys Herringbone_World_Generation and releases owned resources.
    ~Herringbone_World_Generation();

    // Loads tileset from image.
    bool load_tileset_from_image(const char *path);
    // Generates map.
    bool generate_map(const char *output_filename, const int output_width, const int output_height);

    // Returns image data.
    unsigned char *get_image_data();
};

// stbhw_tile genesis_tile = {};
