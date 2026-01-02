#pragma once

#include <glm/glm.hpp>

#include "engine/particle/particle.hpp"

// forwardrd declarations
// class Particle;

struct WorldCell
{
    bool operator==(const WorldCell &other) const
    {
        return coords == other.coords && particle == other.particle;
    }

    glm::ivec2 coords;
    Particle particle;

    WorldCell(glm::ivec2 coords);
    WorldCell(glm::ivec2 coords, Particle particle);
    ~WorldCell() = default;

    bool visited = false;

    void set_visited();
    void set_particle(Particle particle);
};
