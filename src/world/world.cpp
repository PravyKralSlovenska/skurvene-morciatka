#include <iostream>
#include "others/utils.hpp"
#include "engine/world.hpp"

/*
 * WORLD
 */
World::World(int w, int h, int scale)
    : m_rows(w / scale), m_cols(h / scale), seed(1) {}

void World::update_world()
{
    for (size_t i = m_rows; i >= 0; --i)
    {
        for (size_t j = m_cols; j >= 0; --i)
        {
            Particle &particle = world_curr[i * m_cols + j];

            if (particle.state == ParticleState::NONE || particle.type == ParticleType::EMPTY)
            {
                // particle.move();
            }
        }

        for (size_t j = 0; j <= m_cols; ++i)
        {
        }
    }
}