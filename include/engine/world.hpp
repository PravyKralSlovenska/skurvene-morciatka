#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "chunk.hpp"

/*
 * Vertix
 * - reprezentuje jeden particle v svete na GPUcku
 */
struct Vertix
{
    float x, y;
    float r, g, b;

    Vertix(float x, float y, float r, float g, float b) : x(x), y(y), r(r), g(g), b(b) {}
};

/*
 * WorldCell
 * - reprezentuje jednu bunku v svete
 * - obsahuje informacie o tom, ci je bunka prazdna a aky particle sa v nej nachadza
 */
struct WorldCell
{
    int x, y;
    bool empty = true;
    std::shared_ptr<Particle> particle = nullptr;

    void set_particle(std::shared_ptr<Particle> p);
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
    /*
     * WORLD CURRENT
     * - obsahuje aktualny stav sveta
     * - pouziva sa na renderovanie
     * [{SAND, EMPTY, EMPTY}, {EMPTY, EMPTY, SAND}, ...]
     */
    std::vector<std::vector<WorldCell>> world_curr;

    /*
     * WORLD NEXT
     * - obsahuje dalsi stav sveta
     * - pouziva sa na update sveta
     * [{EMPTY, EMPTY, EMPTY}, {SAND, EMPTY, SAND}, ...]
     */
    std::vector<std::vector<WorldCell>> world_next;

    /*
     * VERTIX BUFFER
     * - obsahuje data pre OpenGL na renderovanie
     * - pouziva sa na renderovanie sveta
     * [{x, y, r, g, b}, {x, y, r, g, b}, ...]
     */
    mutable std::vector<Vertix> vertix_buffer;
    mutable bool vertix_buffer_dirty = true;

public:
    int width, height, particle_size;

    World(int w, int h, int ps);
    ~World() = default;

    void add_particle(int particle, int x, int y);
    void remove_particle();
    void update_world();
    void clear_worlds();
    void swap_worlds();
    void swap_particles();
    void render_world();
    void load_chunk();
    void unload_chunk();

    // debug
    void print_world_info();
};
