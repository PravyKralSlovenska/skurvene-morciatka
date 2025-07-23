#pragma once
#include <string>

std::string read_file(const std::string &filepath);

/*
 * v utils.hpp
 * - pomocne funkcia na zistenie ci je WorldCell/Particle validny v mriezke sveta
 */
bool in_world_range(int x, int y, int world_rows, int world_cols);

unsigned int create_shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);
unsigned int compile_shader(unsigned int type, const std::string &source);

unsigned int create_vertex_array_buffer();
unsigned int create_vertex_buffer_object();
unsigned int create_element_buffer_object();

// chcem lepsie pracovat s farbou
// -  upravujem farbu tak aby vedel opengl s tym pracovat
struct Color
{
    float r, g, b, a;

    Color();
    Color(int r, int g, int b, float a);
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
