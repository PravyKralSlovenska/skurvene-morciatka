#include "engine/world/world_cell.hpp"

WorldCell::WorldCell(glm::ivec2 coords)
    : coords(coords), particle(), visited(false) {}

void WorldCell::set_particle(Particle particle)
{
    this->particle = particle;
}

void WorldCell::set_visited()
{
    visited = !visited;
}
