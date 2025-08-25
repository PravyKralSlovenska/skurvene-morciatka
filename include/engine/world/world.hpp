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

    void clear_world();
    
    void add_particle(glm::vec2 coords, Particle_Type type, int size);
    void move_particle(const WorldCell& source_cell, const WorldCell& target_cell);
    void swap_worlds();

    bool can_move(const int x, const int y);
    bool move(WorldCell &worldcell, const Particle_Movement movement);
    glm::vec2 direction_to_offset(Particle_Movement direction);
    
    // void move_solid(WorldCell &worldcell);
    // void move_gas(WorldCell &worldcell);
    // void move_liquid(WorldCell &worldcell);
    // void find_place_to_move(const Particle &particle);

    std::vector<WorldCell> &get_world();
    WorldCell &get_worldcell(int x, int y);
    size_t get_index(int x, int y);
    void print_world();
    void debug_particle(int x, int y);
};