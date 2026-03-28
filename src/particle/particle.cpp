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
    physics.temperature = 25.0f;     // Room temperature in Celsius
    physics.melting_point = 1700.0f; // Sand melts to glass
    physics.boiling_point = 2230.0f;
    physics.max_temperature = 2600.0f;
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
    physics.density = 1000.0f;      // kg/m^3 - water density
    physics.temperature = 25.0f;    // Room temperature in Celsius
    physics.melting_point = 0.0f;   // 0°C - water freezes
    physics.boiling_point = 100.0f; // 100°C - water boils
    physics.max_temperature = 220.0f;
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

Particle create_ice(bool is_static)
{
    Particle_Physics physics;
    physics.density = 917.0f;     // kg/m^3 - ice
    physics.temperature = -10.0f; // Below freezing
    physics.melting_point = 0.0f; // 0°C
    physics.boiling_point = 0.0f; // N/A for this solid
    physics.max_temperature = 60.0f;
    physics.thermal_conductivity = 2.2f; // Higher than water
    physics.specific_heat = 2100.0f;
    physics.viscosity = 0.0f;
    physics.dispersion_rate = 0;

    Particle p(
        Particle_Type::ICE,
        Particle_State::SOLID,
        Particle_Movement::MOVE_SOLID,
        Color(190, 235, 255, 0.95),
        physics);

    p.set_static(is_static);
    return p;
}

Particle create_water_vapor(bool is_static)
{
    Particle_Physics physics;
    physics.density = 0.6f;       // kg/m^3 - light gas
    physics.temperature = 120.0f; // Hot steam
    physics.melting_point = 0.0f;
    physics.boiling_point = 100.0f;
    physics.max_temperature = 450.0f;
    physics.thermal_conductivity = 0.02f;
    physics.specific_heat = 2000.0f;
    physics.viscosity = 0.0f;
    physics.dispersion_rate = 5;

    Particle p(
        Particle_Type::WATER_VAPOR,
        Particle_State::GAS,
        Particle_Movement::MOVE_GAS,
        Color(225, 235, 245, 0.45),
        physics);

    p.set_static(is_static);
    return p;
}

Particle create_smoke(bool is_static)
{
    Particle_Physics physics;
    physics.density = 1.2f;       // kg/m^3 - slightly heavier than air
    physics.temperature = 140.0f; // Hot smoke
    physics.melting_point = 0.0f; // N/A for gas
    physics.boiling_point = 0.0f; // N/A for gas
    physics.max_temperature = 600.0f;
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

Particle create_wood(bool is_static)
{
    Particle_Physics physics;
    physics.density = 700.0f;       // kg/m^3 - typical dry wood
    physics.temperature = 25.0f;    // Room temperature
    physics.melting_point = 600.0f; // Char/pyrolysis starts around 600C
    physics.boiling_point = 0.0f;   // N/A in this simplified model
    physics.max_temperature = 900.0f;
    physics.smoke_point = 320.0f;
    physics.thermal_conductivity = 0.12f;
    physics.specific_heat = 1700.0f;
    physics.viscosity = 0.0f;
    physics.dispersion_rate = 0;

    Particle p(
        Particle_Type::WOOD,
        Particle_State::SOLID,
        Particle_Movement::STATIC,
        Color(139, 94, 60, 1.0),
        physics);

    p.set_static(is_static);
    p.flags.is_flammable = 1;
    return p;
}

Particle create_fire(bool is_static)
{
    Particle_Physics physics;
    physics.density = 0.4f;       // Very light, rises like hot gas
    physics.temperature = 850.0f; // Hot flame core
    physics.melting_point = 0.0f; // N/A
    physics.boiling_point = 0.0f; // N/A
    physics.max_temperature = 1300.0f;
    physics.thermal_conductivity = 0.08f;
    physics.specific_heat = 1200.0f;
    physics.viscosity = 0.0f;
    physics.dispersion_rate = 2;

    Particle p(
        Particle_Type::FIRE,
        Particle_State::GAS,
        Particle_Movement::MOVE_GAS,
        Color(255, 110, 30, 0.9),
        physics);

    p.set_static(is_static);
    p.flags.is_reactive = 1;
    p.flags.is_on_fire = 1;
    p.lifetime = 90;
    return p;
}

Particle create_stone(bool is_static)
{
    Particle_Physics physics;
    physics.density = 2700.0f;       // kg/m^3 - granite density
    physics.temperature = 25.0f;     // Room temperature
    physics.melting_point = 1200.0f; // Stone melts at high temperature
    physics.boiling_point = 3000.0f;
    physics.max_temperature = 3500.0f;
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
    physics.density = 19100.0f;  // kg/m^3 - uranium is VERY dense
    physics.temperature = 25.0f; // Room temperature
    physics.melting_point = 1132.0f;
    physics.boiling_point = 4131.0f;
    physics.max_temperature = 4400.0f;
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