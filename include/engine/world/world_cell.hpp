#pragma once

#include <glm/glm.hpp>
#include "engine/particle/particle.hpp"

struct WorldCell
{
    glm::ivec2 coords;
    Particle particle;

    WorldCell(glm::ivec2);
    ~WorldCell() = default;

    bool visited = false;

    void set_visited();
    void set_particle(Particle particle);
};
