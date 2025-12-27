#pragma once

#include <glm/glm.hpp>

#include "engine/particle/particle.hpp"

// forwardrd declarations
// class Particle;

struct WorldCell
{
    const glm::ivec2 coords;
    Particle particle;

    WorldCell(glm::ivec2 coords);
    WorldCell(glm::ivec2 coords, Particle particle);
    ~WorldCell() = default;

    bool visited = false;

    void set_visited();
    void set_particle(Particle particle);
};
