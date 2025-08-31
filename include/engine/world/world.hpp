#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <map>

#include <glm/glm.hpp>

#include "engine/particle/particle.hpp"
#include "engine/entity.hpp"

struct WorldCell
{
    glm::vec2 coords;
    Particle particle;

    WorldCell(glm::vec2);
    ~WorldCell() = default;

    bool visited = false;

    void set_visited();
    void set_particle(Particle particle);
};

class World
{
private:
    int seed; // seed na generovanie nahodneho sveta?
    std::vector<WorldCell> world_curr;
    std::vector<WorldCell> world_next;
    // std::vector<> active_chunks;

public:
    size_t m_rows, m_cols;
    int scale;
    std::vector<Entity> entities;

public:
    World(int w, int h, int scale);
    ~World() = default;

public:
    void update_world_loop();
    void update_world_decider(int x, int y);

    void clear_world_curr();
    void clear_world_next();

    bool in_world_grid(int x, int y);

    WorldCell &get_worldcell_curr(int x, int y);
    WorldCell &get_worldcell_next(int x, int y);
    WorldCell &get_worldcell_curr(int index);
    WorldCell &get_worldcell_next(int index);

    void add_particle(glm::vec2 coords, Particle_Type type, int size);
    void swap_worlds();
    
    void swap_particles(WorldCell &current_cell, WorldCell &target_cell);
    glm::vec2 direction_to_offset(Particle_Movement direction);

    void move_solid(WorldCell &cell);
    void move_liquid(WorldCell &cell);
    void move_gas(WorldCell &cell);

    std::vector<WorldCell> &get_world_curr();
    std::vector<WorldCell> &get_world_next();

    size_t get_index(int x, int y);
    void print_world();
    void debug_particle(int x, int y);
};