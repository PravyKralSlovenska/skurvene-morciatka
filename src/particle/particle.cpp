#include <iostream>

#include "engine/particle.hpp"
#include "others/utils.hpp"

Particle::Particle() {}

Particle::Particle(ParticleType type, ParticleState state, Color color)
    : type(type), state(state), color(color) {}

Particle create_sand()
{
    return Particle(
        ParticleType::SAND,
        ParticleState::SOLID,
        Color(255, 255, 0, 1.0));
}

Particle create_water()
{
    return Particle(
        ParticleType::WATER,
        ParticleState::LIQUID,
        Color(0, 0, 255, 0.5));
}

Particle create_smoke()
{
    return Particle(
        ParticleType::SMOKE,
        ParticleState::GAS,
        Color(125, 125, 125, 1.0));
}
