#pragma once

#include "engine/particle/particle_density.hpp"

class Particle_Mass
{
public:
    float mass; // kg
    float ONE_SIDE_OF_A_PARTICLE;

public:
    Particle_Mass(Particle_Density *density, float ONE_SIDE_OF_A_PARTICLE);
    ~Particle_Mass() = default;

    void set_mass(float mass);
    float get_mass();

    void change_mass(float delta_m);
    void increase_mass(float delta_m);
    void decrease_mass(float delta_m);
};
