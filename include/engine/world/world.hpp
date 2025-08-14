#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "engine/particle/particle.hpp"
#include "engine/entity.hpp"

struct WorldCell
{
    bool empty = true;
    glm::vec2 coords;
    Particle particle;

    WorldCell(glm::vec2);
    ~WorldCell() = default;

    void set_particle(Particle particle);
};

class World
{
private:
    int seed; // seed na generovanie nahodneho sveta?
    std::vector<WorldCell> world_curr;
    // std::vector<Particle> world_next;
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
    void clear_world();
    void add_particle(glm::vec2 coords, ParticleType type, int size);
    void swap_particles(Particle &this_particle, Particle &that_particle);

    void move_solid(WorldCell &worldcell);
    void move_gas(WorldCell &worldcell);
    void move_liquid(WorldCell &worldcell);
    // void find_place_to_move(const Particle &particle);

    std::vector<WorldCell> &get_world();

    WorldCell &get_worldcell(int x, int y);

    void print_world();
};
