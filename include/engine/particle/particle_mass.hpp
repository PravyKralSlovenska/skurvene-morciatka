#pragma once

// File purpose: Provides mass utilities and mass state for particles.
#include "engine/particle/particle_density.hpp"

// Tracks and updates particle mass values.
class Particle_Mass
{
public:
    float mass; // kg
    float ONE_SIDE_OF_A_PARTICLE;

public:
    // Constructs Particle_Mass.
    Particle_Mass(Particle_Density *density, float ONE_SIDE_OF_A_PARTICLE);
    // Destroys Particle_Mass and releases owned resources.
    ~Particle_Mass() = default;

    // Sets mass.
    void set_mass(float mass);
    // Returns mass.
    float get_mass();

    // Changes mass.
    void change_mass(float delta_m);
    // Increases mass.
    void increase_mass(float delta_m);
    // Decreases mass.
    void decrease_mass(float delta_m);
};
