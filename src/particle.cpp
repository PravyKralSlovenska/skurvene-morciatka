#include <iostream>
#include <array>
#include <memory>

#include "engine/particle.hpp"

Particle::Particle() {}

Particle::Particle(
    std::string name,
    ParticleType type,
    ParticleState state,
    int x,
    int y,
    float vx,
    float vy,
    float density,
    float mass,
    int lifetime,
    float temperature,
    float i_temperature,
    float m_point,
    float b_point,
    bool flammable,
    bool corrosive,
    bool conductive,
    bool magnetic,
    bool source_of_light,
    bool radioactive,
    bool explosive,
    bool toxic,
    std::array<int, 3> color,
    float opacity)
{
    this->name = name;
    this->state = state;
    this->type = type;
    this->x = x;
    this->y = y;
    this->velocity_x = vx;
    this->velocity_y = vy;
    this->density = density;
    this->mass = mass;
    this->lifetime = lifetime;
    this->temperature = temperature;
    this->ignition_temperature = i_temperature;
    this->melting_point = m_point;
    this->boiling_point = b_point;
    this->color = color;
    this->opacity = opacity;
    this->flammable = flammable;
    this->corrosive = corrosive;
    this->conductive = conductive;
    this->magnetic = magnetic;
    this->source_of_light = source_of_light;
    this->radioactive = radioactive;
    this->explosive = explosive;
    this->toxic = toxic;
}

Particle Particle::create_sand(int x, int y, float vx, float vy)
{
    return Particle(
        "SAND",
        ParticleType::SAND,
        ParticleState::SOLID,
        x, y,
        vx, vy,
        1442.0f,
        1.0f,
        1000,
        20.0f,
        500.f,
        1700.0f,
        2230.0f,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        std::array<int, 3>{255, 255, 125},
        1.0f);
}

Particle Particle::create_water(int x, int y, float vx, float vy)
{
    return Particle(
        "WATER",
        ParticleType::WATER,
        ParticleState::LIQUID,
        x, y,
        vx, vy,
        997.0f,       // d
        1.0f,         // m
        100,          // lifetime
        4.0f,         // temperature
        999999999.0f, // ignition temperature
        0.0f,         // melting point
        100.0f,       // boiling point
        false,
        false,
        true, // conductive
        false,
        false,
        false,
        false,
        false,
        std::array<int, 3>{0, 125, 255},
        1.0f);
}

Particle Particle::create_smoke(int x, int y, float vx, float vy)
{
    return Particle(
        "SMOKE",
        ParticleType::SMOKE,
        ParticleState::GAS,
        x, y,
        vx, vy,
        0.02f,        // d
        1.0f,         // m
        100,          // lifetime
        120.0f,       // temperature
        350.0f,       // ignition temperature
        999999999.0f, // melting point
        999999999.0f, // boiling point
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        std::array<int, 3>{125, 125, 125},
        0.5f);
}
