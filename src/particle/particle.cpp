#include "engine/particle/particle.hpp"

Particle::Particle() {}

Particle::Particle(Particle_Type type, Particle_State state, Particle_Movement move, Color base_color)
    : type(type), state(state), move(move), base_color(base_color)
{
    color = base_color;
}

Particle create_sand()
{
    return Particle(
        Particle_Type::SAND,
        Particle_State::SOLID,
        Particle_Movement::MOVE_SOLID,
        Color(203, 189, 0, 1.0));
}

Particle create_water()
{
    return Particle(
        Particle_Type::WATER,
        Particle_State::LIQUID,
        Particle_Movement::DOWN 
            | Particle_Movement::DOWN_LEFT 
            | Particle_Movement::DOWN_RIGHT 
            | Particle_Movement::LEFT 
            | Particle_Movement::RIGHT,
        Color(0, 0, 255, 0.5));
}

Particle create_smoke()
{
    return Particle(
        Particle_Type::SMOKE,
        Particle_State::GAS,
        Particle_Movement::UP 
            | Particle_Movement::UP_LEFT 
            | Particle_Movement::UP_RIGHT 
            | Particle_Movement::LEFT 
            | Particle_Movement::RIGHT, 
        Color(125, 125, 125, 0.5));
}

Particle create_stone()
{
    return Particle(
        Particle_Type::STONE,
        Particle_State::SOLID,
        Particle_Movement::NONE,
        Color(128, 128, 128, 1.0)  
    );
}