#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "others/utils.hpp"

Color::Color() {}
Color::Color(int r, int g, int b, float a)
    : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a) {}

/*
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