#include "engine/world/world_cell.hpp"

WorldCell::WorldCell(glm::ivec2 coords)
    : coords(coords), particle(), visited(false) {}

WorldCell::WorldCell(glm::ivec2 coords, Particle particle)
    : coords(coords), particle(particle) {}

void WorldCell::set_particle(Particle particle)
{
    this->particle = particle;    // ...existing code...
    // ...existing code...
}

void WorldCell::set_visited()
{
    visited = !visited;
}
