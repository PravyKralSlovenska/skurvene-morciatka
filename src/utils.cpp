#include "others/utils.hpp"

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "others/utils.hpp"

Color::Color() {}
Color::Color(int red, int green, int blue, float alpha)
    : r(red / 255.0f), g(green / 255.0f), b(blue / 255.0f), a(alpha)
{
}

Color Color::change_shade()
{
    // float shader_factor = 0.1f + (static_cast<float>(rand()) / RAND_MAX) * 0.05f;
    float shader_factor = 0.1f + (static_cast<float>(rand()) / RAND_MAX) * 0.15f;

    int red = r * 255;
    int green = g * 255;
    int blue = b * 255;

    int new_red = red * (1 - shader_factor);
    int new_green = green * (1 - shader_factor);
    int new_blue = blue * (1 - shader_factor);

    return Color(new_red, new_green, new_blue, a);
}

Color Color::change_tint()
{
    float tint_factor;

    return Color();
}

Vertex::Vertex() {}
Vertex::Vertex(float x, float y, Color color)
    : x(x), y(y), color(color) {}

/*
 * z utils.hpp
 *
 */
std::string read_file(const std::string &filepath)
{
    // std::string font_name = FileSystem::getPath();
    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

/*
 * z utils.hpp
 * Skontroluje, ci je hodnota v rozsahu
 * Pouzivam v particles.hpp
 */
bool in_world_range(int x, int y, int world_rows, int world_cols)
{
    return (x >= 0 && x < world_cols) && (y >= 0 && y < world_rows);
}

// Random::Random()
//     : gen(rand_device()) {}

std::random_device Random_Machine::rand_device;
std::mt19937 Random_Machine::gen(Random_Machine::rand_device());

int Random_Machine::get_int_from_range(int start, int end)
{
    std::uniform_int_distribution<> distrib(start, end);
    return distrib(gen);
}

float Random_Machine::get_float()
{
    std::uniform_real_distribution<float> distrib(0.0f, 1.0f);
    return distrib(gen);
}

/*
 * z utils.hpp
 * vrati vector
 */
std::vector<glm::ivec2> calculate_offsets(const int radius)
{
    std::vector<glm::ivec2> offsets;

    for (int i = -radius; i <= radius; i++)
        for (int j = -radius; j <= radius; j++)
        {
            float distance_from_player = std::sqrt(i * i + j * j);
            if (distance_from_player <= radius)
            {
                offsets.push_back({j, i});
            }
        }

    return offsets;
}

std::vector<glm::ivec2> calculate_offsets_square(const int radius)
{
    std::vector<glm::ivec2> offsets;
    offsets.reserve((2 * radius + 1) * (2 * radius + 1));

    for (int i = -radius; i <= radius; i++)
        for (int j = -radius; j <= radius; j++)
        {
            offsets.push_back({j, i});
        }

    return offsets;
}

int hash_coords(int x, int y, int seed)
{
    return ((x * 374761393) + (y * 668265263) + seed) % 1000000007;
}
