#pragma once

#include <random>
#include <vector>
#include <glm/glm.hpp>

std::string read_file(const std::string &filepath);

/*
 * v kruznici
 */
std::vector<glm::ivec2> calculate_offsets(const int radius);

/*
 * v stvorci
 */
std::vector<glm::ivec2> calculate_offsets_square(const int radius);

/*
 * deterministicke
 */
int hash_coords(int x, int y, int seed);

/*
 * v utils.hpp
 * - pomocne funkcia na zistenie ci je WorldCell/Particle validny v mriezke sveta
 */
bool in_world_range(int x, int y, int world_rows, int world_cols);

class Random_Machine
{
private:
    static std::random_device rand_device; // thread_local ?
    static std::mt19937 gen;

private:
    Random_Machine() = delete;
    Random_Machine(const Random_Machine &) = delete;
    Random_Machine &operator=(const Random_Machine &) = delete;
    ~Random_Machine() = delete;

public:
    static int get_int_from_range(int start, int end);
    static float get_float();

    template <typename T, std::size_t N>
    static T get_random_element_from_array(const std::array<T, N> &array)
    {
        int index = get_int_from_range(0, array.size() - 1);
        return array[index];
    }

    template <typename T>
    static T get_random_element_from_vector(const std::vector<T> &vector)
    {
        if (vector.empty())
        {
            return -1;
        }

        int index = get_int_from_range(0, vector.size() - 1);
        return vector[index];
    }
};

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
    const std::string path_to_log_folder = "/log";

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