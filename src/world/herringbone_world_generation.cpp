#include "engine/world/herringbone_world_generation.hpp"

#include <iostream>

#include "others/utils.hpp"

// #define STB_HBWANG_RAND() Random_Machine::get_int_from_range(0, 200)
// #define STB_HBWANG_MAX_X 10000
// #define STB_HBWANG_MAX_Y 10000
#define STB_HERRINGBONE_WANG_TILE_IMPLEMENTATION
#include "stb/stb_herringbone_wang_tile.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

Herringbone_World_Generation::Herringbone_World_Generation(const int seed)
    : seed(seed) {}

Herringbone_World_Generation::~Herringbone_World_Generation()
{
    delete[] image_data_buffer;
    if (&tileset)
    {
        stbhw_free_tileset(&tileset);
    }
}

bool Herringbone_World_Generation::load_tileset_from_image(const char *path)
{
    int width, height;
    unsigned char *image_data = stbi_load(path, &width, &height, nullptr, 3);

    if (!image_data)
    {
        std::cerr << "HW ERROR: load a image " << path << '\n';
        return false;
    }

    if (!stbhw_build_tileset_from_image(&tileset, image_data, width * 3, width, height))
    {
        std::cerr << "HW ERROR: getting a tileset\n";
        return false;
    }

    stbi_image_free(image_data);

    std::cout << "HW INFO: uspech!\n";
    return true;
}

bool Herringbone_World_Generation::generate_map(const char *output_filename, const int output_width, const int output_height)
{
    image_data_buffer = (unsigned char *)malloc(3 * output_width * output_height);
    stbhw_generate_image(&tileset, nullptr, image_data_buffer, output_width * 3, output_width, output_height);

    if (stbi_write_png(output_filename, output_width, output_height, 3, image_data_buffer, output_width * 3))
    {
        std::cout << "Map saved to: " << output_filename << '\n';
    }
    else
    {
        std::cerr << "Failed to save map to: " << output_filename << '\n';
    }

    return true;
}

unsigned char *Herringbone_World_Generation::get_image_data()
{
    return image_data_buffer;
}
