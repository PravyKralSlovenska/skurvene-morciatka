#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "particles.hpp"

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

struct WorldCell
{
    // shared_ptr je 
    // nepotrebujem volat delete, ked sa WorldCell nebudem pouzivat
    std::shared_ptr<Particle> particle;
};

// zatial nie
class Chunk
{
};

class World
{
private:
    /*
     * WORLD CURRENT
     * - obsahuje aktualny stav sveta
     * - pouziva sa na renderovanie
     * [{x, y, r, g, b}, {x, y, r, g, b}, ...]
     */
    std::vector<WorldCell> world_curr;

    /*
     * WORLD NEXT
     * - obsahuje dalsi stav sveta
     * - pouziva sa na update sveta
     * [{x, y, r, g, b}, {x, y, r, g, b}, ...]
     */
    std::vector<WorldCell> world_next;

    /*
     * VERTIX BUFFER
     * - obsahuje data pre OpenGL na renderovanie
     * - pouziva sa na renderovanie sveta
     * [{x, y, r, g, b}, {x, y, r, g, b}, ...]
     */
    // mutable std::vector<Vertix> vertix_buffer;
    // mutable bool vertix_buffer_dirty = true;

public:
    const int width;
    const int height;
    const int particle_size;

    World(int width, int height, int particle_size) : width(width),
                                                      height(height),
                                                      particle_size(particle_size)
    {
        world_curr.resize((width / particle_size) * (height / particle_size));
        world_next.resize((width / particle_size) * (height / particle_size));
    }
    ~World() = default;

    void add_particle();
    void update_world();
    void render_world();
    void swap_worlds();
    void load_chunk();
    void unload_chunk();
};
