#include "engine/particle/particle_velocity.hpp"

Particle_Velocity::Particle_Velocity(Particle_Density *density, Particle_Mass *mass, float GRAVITY)
    : density(density), mass(mass), GRAVITY(GRAVITY)
{
    calculate_terminal_velocity();
}

void Particle_Velocity::calculate_terminal_velocity()
{
    terminal_velocity = (int)sqrt((2 * mass->mass * GRAVITY) / (enviroment_density * (mass->ONE_SIDE_OF_A_PARTICLE * mass->ONE_SIDE_OF_A_PARTICLE))); // treba dokoncit
}

void Particle_Velocity::set_environment_density(float enviroment_density)
{
    this->enviroment_density = enviroment_density;
}

glm::vec2 Particle_Velocity::get_velocity()
{
    return velocity;
}

float Particle_Velocity::get_velocity_x()
{
    return velocity.x;
}

float Particle_Velocity::get_velocity_y()
{
    return velocity.y;
}

void Particle_Velocity::set_velocity(glm::vec2 velocity)
{
    this->velocity = velocity;
}

void Particle_Velocity::set_velocity_x(float velocity_x)
{
    velocity.x = velocity_x;
}

void Particle_Velocity::set_velocity_y(float velocity_y)
{
    velocity.y = velocity_y;
}

void Particle_Velocity::change_velocity(glm::vec2 delta_velocity)
{
    velocity += delta_velocity;
}

void Particle_Velocity::change_velocity_x(float delta_velocity_x)
{
    velocity.x += delta_velocity_x;
}

void Particle_Velocity::change_velocity_y(float delta_velocity_y)
{
    velocity.y += delta_velocity_y;
}
