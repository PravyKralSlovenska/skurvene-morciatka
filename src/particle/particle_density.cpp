#include "engine/particle/particle_density.hpp"

Particle_Density::Particle_Density(float density)
{
    this->density = density * 0.01; // km/m^3 -> km/m^2
}

Particle_Density::~Particle_Density() {}