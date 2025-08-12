#include "engine/particle/particle_movement.hpp"

std::map<std::string, Particle &> Particle_Movement::find_places(World *world, Particle *particle)
{
    std::vector<glm::ivec2> directions;

    switch (particle->state)
    {
    case ParticleState::SOLID:
        directions = {{1, -1}, {0, -1}, {-1, -1}};
        break;
    case ParticleState::LIQUID:
        directions = {{1, 0}, {-1, 0}, {1, -1}, {0, -1}, {-1, -1}};
        break;
    case ParticleState::GAS:
        directions = {{1, 1}, {0, 1}, {-1, 1}, {1, 0}, {-1, 0}};
        break;
    default:
        break;
    }

    return find_moore_neighborhood(world, particle, directions);
}

std::map<std::string, Particle &> Particle_Movement::find_moore_neighborhood(World *world, Particle *particle, const std::vector<glm::ivec2> &neighborhood_coords)
{
    std::map<std::string, Particle &> neighbors;

    auto x = particle->coords.x;
    auto y = particle->coords.y;

    for (const auto coords : neighborhood_coords)
    {
        int dx = x + coords.x;
        int dy = y + coords.y;

        if (!in_world_range(dx, dy, world->m_rows, world->m_cols))
        {
            continue;
        }

        Particle &neighbor = world->get_particle(dx, dy);

        if (neighbor.type == ParticleType::EMPTY)
        {
            // neighbors.push_back(neighbor);
            neighbors[neighbors_map[coords]] = neighbor;
        }
    }

    return neighbors;
}

void Particle_Movement::move(Particle *this_particle, Particle *other_particle)
{
}

void Particle_Movement::move_solid(World *world, Particle *particle)
{
    auto neighbors = find_places(world, particle);
    if (neighbors.empty())
    {
        return;
    }

    if (neighbors.count("down"))
    {
        move(particle, &neighbors["down"]);
    }
    else if (neighbors.count("down_left") && neighbors.count("down_right"))
    {
        if (rand() % 2)
        {
            move(particle, &neighbors["down_left"]);
        }
        else
        {
            move(particle, &neighbors["down_right"]);
        }
    }
    else if (neighbors.count("down_left"))
    {
        move(particle, &neighbors["down_left"]);
    }
    else if (neighbors.count("down_right"))
    {
        move(particle, &neighbors["down_right"]);
    }
}

void Particle_Movement::move_liquid(World *world, Particle *particle)
{
    auto neighbors = find_places(world, particle);
    if (neighbors.empty())
    {
        return;
    }

    if (neighbors.count("down"))
    {
        move(particle, &neighbors["down"]);
    }
    else if (neighbors.count("down_left") && neighbors.count("down_right"))
    {
        if (rand() % 2)
        {
            move(particle, &neighbors["down_left"]);
        }
        else
        {
            move(particle, &neighbors["down_right"]);
        }
    }
    else if (neighbors.count("down_left"))
    {
        move(particle, &neighbors["down_left"]);
    }
    else if (neighbors.count("down_right"))
    {
        move(particle, &neighbors["down_right"]);
    }
    // najdi miesto
}

void Particle_Movement::move_gas(World *world, Particle *particle)
{
}

void whatever(World *world, Particle *particle)
{
    auto x = particle->coords.x;
    auto y = particle->coords.y;

    size_t left = y - (world->m_cols - y);
    size_t right = world->m_cols - y;
}