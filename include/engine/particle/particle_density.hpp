#pragma once

// File purpose: Provides particle density values used by simulation rules.
// Provides density constants and comparisons for particles.
class Particle_Density
{
public:
    float density; // kg/m^2

public:
    // Constructs Particle_Density.
    Particle_Density(float density);
    // Destroys Particle_Density and releases owned resources.
    ~Particle_Density();
};
