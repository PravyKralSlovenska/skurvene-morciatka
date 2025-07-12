#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <optional>
#include "chunk.hpp"
#include "particle.hpp"

/*
 * Vertex
 * - reprezentuje jeden particle v svete na GPUcku
 */
struct Vertex
{
    float x, y;
    float r, g, b, a; // urobil som ze musis zadat hodnotu do 255 a program si to uz upravi podla seba

    Vertex();
    Vertex(float x, float y, float r, float g, float b, float a);
    ~Vertex() = default;
};

/*
 * WorldCell
 * - reprezentuje jednu bunku v svete
 * - obsahuje informacie o tom, ci je bunka prazdna a aky particle sa v nej nachadza
 */
struct WorldCell
{
    int x, y;
    std::optional<Particle> particle;

    bool empty = true;
    bool visited = false;

    WorldCell();
    WorldCell(int x, int y);
    ~WorldCell() = default;

    std::string get_info();
    void set_particle(Particle p);
    void set_empty();
};

/*
 * World
 * - in real time reprezentacia sveta
 * - WORLD je moj physics engine
 */
class World
{
private:
    int seed; // seed na generovanie nahodneho sveta?

    /*
     * WORLD CURRENT
     * - obsahuje aktualny stav sveta
     * - pouziva sa na renderovanie
     * [SAND, EMPTY, EMPTY, EMPTY, EMPTY, SAND, ...]
     */
    std::vector<WorldCell> world_curr;

    /*
     * WORLD NEXT
     * - obsahuje dalsi stav sveta
     * - pouziva sa na update sveta
     * [SAND, EMPTY, EMPTY, EMPTY, EMPTY, SAND, ...]
     */
    std::vector<WorldCell> world_next;

    /*
     * Vertex BUFFER
     * - obsahuje data pre OpenGL na renderovanie
     * - pouziva sa na renderovanie sveta
     * [{x, y, r, g, b}, {x, y, r, g, b}, ...]
     */
    mutable bool vertex_buffer_dirty = true;

public:
    int width, height, particle_size;

    // nezabudni odcitat jeden index -1
    int rows, cols;

    World(int w, int h, int ps);
    ~World() = default;

    std::vector<Vertex> vertex_buffer;
    std::vector<unsigned int> indices;

    void add_particle(Particle particle, int x, int y);
    void remove_particle();
    void update_world();
    void clear_worlds();

    void push_vertex_buffer(Particle &particle, int grid_x, int grid_y);
    void clear_vertex_buffer();

    void swap_worlds();
    void swap_particles(WorldCell &worldcell1, WorldCell &worldcell2);

    // napln vertex_buffer a potom ho vrat na renderovanie do openGL
    void render_world();

    void load_chunk();
    void unload_chunk();

    void update_temperature();
    void apply_gravity();

    // pohyb particles
    void move_solid(WorldCell &worldcell);
    void move_liquid(WorldCell &worldcell);
    void move_gas(WorldCell &worldcell);

    // debug
    void print_world_info();
    void log_world();
};
