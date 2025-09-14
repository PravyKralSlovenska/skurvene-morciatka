#include "engine/particle/particle_mass.hpp"

Particle_Mass::Particle_Mass(Particle_Density *density, float ONE_SIDE_OF_A_PARTICLE)
    : ONE_SIDE_OF_A_PARTICLE(ONE_SIDE_OF_A_PARTICLE)
{
    mass = density->density * (ONE_SIDE_OF_A_PARTICLE * ONE_SIDE_OF_A_PARTICLE);
}

void Particle_Mass::set_mass(float mass)
{
    this->mass = mass;
}

float Particle_Mass::get_mass()
{
    return mass;
}

void Particle_Mass::change_mass(float delta_m)
{
    mass += delta_m;
}

void Particle_Mass::increase_mass(float delta_m)
{
    mass += delta_m;
}

void Particle_Mass::decrease_mass(float delta_m)
{
    mass -= delta_m;
}