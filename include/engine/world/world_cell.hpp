#pragma once

#include <glm/glm.hpp>
#include "engine/particle/particle.hpp"

struct WorldCell
{
    const glm::ivec2 coords;
    Particle particle;

    WorldCell(glm::ivec2);
    WorldCell(glm::ivec2, Particle particle);
    ~WorldCell() = default;

    bool visited = false;

    void set_visited();
    void set_particle(Particle particle);
};
