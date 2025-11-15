#pragma once

#include <string>
#include <random>
#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

class Random
{
private:
    std::random_device rand_device;
    std::mt19937 gen;

public:
    Random();
    ~Random() = default;

    int get_int_from_range(int start, int end);
    int get_truly_random_int();

    template <typename T, std::size_t N>
    T get_random_element_from_array(const std::array<T, N> &array)
    {
        int index = get_int_from_range(0, array.size() - 1);
        return array[index];
    }

    template <typename T>
    T get_random_element_from_vector(const std::vector<T> &vector)
    {
        if (vector.empty())
        {
            return -1;
        }

        int index = get_int_from_range(0, vector.size() - 1);
        return vector[index];
    }
};

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

    Color change_shade();
    Color change_tint();
};

// enum Color_Enum
// {
//     RED = Color(255, 0, 0, 1.0f),
//     GREEN = Color(0, 255, 0, 1.0f)
// };

class Vertex
{
public:
    float x, y;
    Color color;

public:
    Vertex();
    Vertex(float x, float y, Color color);
    ~Vertex() = default;

    // int get_stride();
    // int get_
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

// mozno nieco na threads? management multithreadov?
class Thread_Manager
{
private:
    unsigned int id;
    //  thread thread_local;

private:
    void create_thread();
    void join_thread();
    void detach_thread();
};