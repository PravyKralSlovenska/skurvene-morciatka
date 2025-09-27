#include "engine/world/herringbone_world_generation.hpp"

#define STB_HERRINGBONE_WANG_TILE_IMPLEMENTATION
#define STB_HBWANG_RAND() Random().get_int_from_range(1, 128)
#include "stb/stb_herringbone_wang_tile.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

Herringbone_World_Generation::Herringbone_World_Generation(int seed)
    : seed(seed) {}

Herringbone_World_Generation::~Herringbone_World_Generation()
{
    if (&tileset)
    {
        stbhw_free_tileset(&tileset);
    }
}

bool Herringbone_World_Generation::load_tileset_from_image(const char *path)
{
    int width, height, channels;
    unsigned char *image_data = stbi_load(path, &width, &height, &channels, 3);

    if (!image_data)
    {
        std::cerr << "HW ERROR: load a image " << path << '\n';
        return false;
    }

    int rv = stbhw_build_tileset_from_image(&tileset, image_data, width * 3, width, height);

    stbi_image_free(image_data);

    if (rv != 1)
    {
        std::cerr << "HW ERROR: getting a tileset\n";
        return false;
    }

    std::cout << "HW INFO: uspech!\n";
    return true;
}

bool Herringbone_World_Generation::generate_map(const char *output_filename, int output_width, int output_height)
{
    unsigned char *image_data_buffer = new unsigned char[output_width * output_height * 3];
    stbhw_generate_image(&tileset, nullptr, image_data_buffer, output_width * 3, output_width, output_height);

    if (stbi_write_png(output_filename, output_width, output_height, 3, image_data_buffer, output_width * 3))
    {
        std::cout << "Map saved to: " << output_filename << '\n';
    }
    else
    {
        std::cerr << "Failed to save map to: " << output_filename << '\n';
    }

    delete[] image_data_buffer;

    return true;
}
