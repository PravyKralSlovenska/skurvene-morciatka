#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "engine/particle/particle.hpp"
#include "engine/entity.hpp"

class World
{
private:
    int seed; // seed na generovanie nahodneho sveta?
    std::vector<Particle> world_curr;
    std::vector<Particle> world_next;
    // std::vector<> active_chunks;

public:
    size_t m_rows, m_cols;
    int scale;
    std::vector<Entity> entities;

public:
    World(int w, int h, int scale);
    ~World() = default;

public:
    void update_world();
    void add_particle(glm::vec2 coords, ParticleType type);
    void swap_particles(Particle &this_particle, Particle &that_particle);

    void move_solid(Particle &particle);
    void move_gas(Particle &particle);
    void move_liquid(Particle &particle);
    void find_place_to_move(const Particle &particle);

    std::vector<Particle> &get_world();

    Particle& get_particle(int x, int y);

    void print_world();
};
