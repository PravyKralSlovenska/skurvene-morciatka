#include "engine/particle/particle.hpp"

// Particle_Physics implementation
Particle_Physics::Particle_Physics(float density, float temperature, float melting_point, float boiling_point)
    : density(density), temperature(temperature), melting_point(melting_point), boiling_point(boiling_point)
{
}

// Particle implementation
Particle::Particle() {}

Particle::Particle(Particle_Type type, Particle_State state, Particle_Movement move, Color base_color)
    : type(type), state(state), move(move), base_color(base_color)
{
    color = base_color;
    if (type != Particle_Type::EMPTY)
    {
        color = base_color.change_shade();
    }
}

Particle::Particle(Particle_Type type, Particle_State state, Particle_Movement move, Color base_color, Particle_Physics physics)
    : type(type), state(state), move(move), base_color(base_color), physics(physics)
{
    color = base_color;
    if (type != Particle_Type::EMPTY)
    {
        color = base_color.change_shade();
    }
}

Particle create_empty()
{
    return Particle();
}

Particle create_sand(bool is_static)
{
    Particle_Physics physics;
    physics.density = 1600.0f;       // kg/m^3 - sand density
    physics.temperature = 293.0f;    // Room temperature in Kelvin
    physics.melting_point = 1973.0f; // ~1700°C - sand melts to glass
    physics.boiling_point = 2503.0f; // ~2230°C
    physics.thermal_conductivity = 0.25f;
    physics.specific_heat = 830.0f; // J/(kg·K)
    physics.viscosity = 0.0f;       // Not applicable for solid
    physics.dispersion_rate = 0;

    Particle p(
        Particle_Type::SAND,
        Particle_State::SOLID,
        Particle_Movement::MOVE_SOLID,
        Color(194, 178, 128, 1.0),
        physics);

    p.set_static(is_static);
    return p;
}

Particle create_water(bool is_static)
{
    Particle_Physics physics;
    physics.density = 1000.0f;           // kg/m^3 - water density
    physics.temperature = 293.0f;        // Room temperature in Kelvin
    physics.melting_point = 273.0f;      // 0°C - water freezes
    physics.boiling_point = 373.0f;      // 100°C - water boils
    physics.thermal_conductivity = 0.6f; // W/(m·K)
    physics.specific_heat = 4186.0f;     // J/(kg·K) - high heat capacity
    physics.viscosity = 0.001f;          // Low viscosity - flows easily
    physics.dispersion_rate = 5;         // Spreads horizontally quite fast

    Particle p(
        Particle_Type::WATER,
        Particle_State::LIQUID,
        Particle_Movement::MOVE_LIQUID,
        Color(30, 144, 255, 0.7),
        physics);

    p.set_static(is_static);
    return p;
}

Particle create_smoke(bool is_static)
{
    Particle_Physics physics;
    physics.density = 1.2f;       // kg/m^3 - slightly heavier than air
    physics.temperature = 400.0f; // Hot smoke
    physics.melting_point = 0.0f; // N/A for gas
    physics.boiling_point = 0.0f; // N/A for gas
    physics.thermal_conductivity = 0.025f;
    physics.specific_heat = 1000.0f;
    physics.viscosity = 0.0f;
    physics.dispersion_rate = 3;

    Particle p(
        Particle_Type::SMOKE,
        Particle_State::GAS,
        Particle_Movement::MOVE_GAS,
        Color(50, 50, 50, 0.6),
        physics);

    p.set_static(is_static);
    p.lifetime = 300; // Smoke dissipates over time
    return p;
}

Particle create_stone(bool is_static)
{
    Particle_Physics physics;
    physics.density = 2700.0f;           // kg/m^3 - granite density
    physics.temperature = 293.0f;        // Room temperature
    physics.melting_point = 1473.0f;     // ~1200°C - stone melts to lava
    physics.boiling_point = 3273.0f;     // ~3000°C
    physics.thermal_conductivity = 2.5f; // Good heat conductor
    physics.specific_heat = 790.0f;
    physics.viscosity = 0.0f;
    physics.dispersion_rate = 0;

    Particle p(
        Particle_Type::STONE,
        Particle_State::SOLID,
        Particle_Movement::STATIC, // Stone doesn't fall
        Color(128, 128, 128, 1.0),
        physics);

    p.set_static(is_static);
    return p;
}

Particle create_uranium(bool is_static)
{
    Particle_Physics physics;
    physics.density = 19100.0f;           // kg/m^3 - uranium is VERY dense
    physics.temperature = 293.0f;         // Room temperature
    physics.melting_point = 1405.0f;      // ~1132°C
    physics.boiling_point = 4404.0f;      // ~4131°C
    physics.thermal_conductivity = 27.5f; // Good heat conductor
    physics.specific_heat = 116.0f;       // Low specific heat
    physics.viscosity = 0.0f;
    physics.dispersion_rate = 0;

    Particle p(
        Particle_Type::URANIUM,
        Particle_State::SOLID,
        Particle_Movement::STATIC, // Uranium ore doesn't fall
        Color(60, 255, 73, 1.0),
        physics);

    p.set_static(is_static);
    p.flags.is_reactive = 1; // Radioactive!
    return p;
}