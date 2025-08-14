#include "engine/particle/particle.hpp"

Particle::Particle() {}

Particle::Particle(ParticleType type, ParticleState state, Color base_color)
    : type(type), state(state), base_color(base_color)
{
    color = base_color;
}

Particle create_sand()
{
    return Particle(
        ParticleType::SAND,
        ParticleState::SOLID,
        Color(203, 189, 0, 1.0));
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
