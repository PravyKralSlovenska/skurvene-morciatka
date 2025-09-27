#include "others/utils.hpp"

Color::Color() {}
Color::Color(int red, int green, int blue, float alpha)
    : r(red / 255.0f), g(green / 255.0f), b(blue / 255.0f), a(alpha)
    {}

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

Random::Random()
: gen(rand_device()) {}

int Random::get_int_from_range(int start, int end)
{
    std::uniform_int_distribution<> distrib(start, end);
    return distrib(gen);
}