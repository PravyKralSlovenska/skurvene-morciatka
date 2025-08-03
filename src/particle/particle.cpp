#include "engine/particle/particle.hpp"

Particle::Particle(glm::vec2 coords)
    : coords(coords) {}

Particle::Particle(ParticleType type, ParticleState state, Color color, glm::vec2 coords)
    : type(type), state(state), color(color), coords(coords) {}

Particle create_sand(glm::vec2 coords)
{
    return Particle(
        ParticleType::SAND,
        ParticleState::SOLID,
        Color(203, 189, 0, 1.0),
        coords);
}

Particle create_water(glm::vec2 coords)
{
    return Particle(
        ParticleType::WATER,
        ParticleState::LIQUID,
        Color(0, 0, 255, 0.5),
        coords);
}

Particle create_smoke(glm::vec2 coords)
{
    return Particle(
        ParticleType::SMOKE,
        ParticleState::GAS,
        Color(125, 125, 125, 1.0),
        coords);
}
