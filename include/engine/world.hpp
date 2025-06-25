#include <iostream>
#include <vector>
#include <memory>

#include "particles.hpp"
#pragma once

class World
{
public:
    int width;
    int height;
    int particle_size;
    std::vector<std::unique_ptr<Particle>> world;
    std::vector<std::unique_ptr<Particle>> world_next;

    World(int width, int height, int particle_size) : width(width),
                                                      height(height),
                                                      particle_size(particle_size)
    {
        world.resize(width * height);
        world_next.resize(width * height);
    }
    ~World() = default;

    void add_particle();
    void update_world();
    void render_world();
    void swap_worlds();
};

