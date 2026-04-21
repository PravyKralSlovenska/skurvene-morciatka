#pragma once

// File purpose: Defines one CPU-side world cell and its particle payload.
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

    // Constructs WorldCell.
    WorldCell(glm::ivec2 coords);
    // Constructs WorldCell.
    WorldCell(glm::ivec2 coords, Particle particle);
    // Destroys WorldCell and releases owned resources.
    ~WorldCell() = default;

    bool visited = false;

    // Sets visited.
    void set_visited();
    // Sets particle.
    void set_particle(Particle particle);
};
