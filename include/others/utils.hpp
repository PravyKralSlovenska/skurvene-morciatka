#pragma once
#include <string>

std::string read_file(const std::string &filepath);

/*
 * v utils.hpp
 * - pomocne funkcia na zistenie ci je WorldCell/Particle validny v mriezke sveta
 */
bool in_world_range(int x, int y, int world_rows, int world_cols);

// chcem lepsie pracovat s farbou
// -  upravujem farbu tak aby vedel opengl s tym pracovat
struct Color
{
    float r, g, b, a;

    Color();
    Color(int red, int green, int blue, float alpha);
};

struct Vertex
{
    float x, y;
    Color color;

    Vertex();
    Vertex(float x, float y, Color color);
    ~Vertex() = default;
};

class Log
{
private:
    std::string path_to_log_folder = "/log";

public:
    Log();
    ~Log() = default;

    void create_file();
    void log_text();
};
