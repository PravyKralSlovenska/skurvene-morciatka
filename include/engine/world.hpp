#pragma once
#include <iostream>
#include <vector>
#include "chunk.hpp"
#include "particle.hpp"

class World
{
private:
    int seed; // seed na generovanie nahodneho sveta?
    std::vector<Particle> world_curr;
    std::vector<Particle> world_next;

public:
    size_t m_rows, m_cols;

public:
    World(int w, int h, int scale);
    ~World() = default;

public:
    int return_index(int x, int y);
    Particle &get_particle(int index);

    void update_world();
};
