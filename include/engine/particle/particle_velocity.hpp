#pragma once

// File purpose: Defines particle velocity helpers and state updates.
#include "engine/particle/particle_mass.hpp"
#include "engine/particle/particle_density.hpp"
#include <glm/glm.hpp>

/*
 * Particle Velocity
 * - zrychlenie alebo rychlost 1.0f znamena ze za 1. sekundu prejde 10 stvorcekov/WorldCellov // toto neviem kde vzniko lol??
 * - samozrejme casom zrychluje pokym nedosiahne "terminal velocity"
 */
// Stores per-particle velocity and velocity helpers.
class Particle_Velocity
{
private:
    Particle_Density *density;
    Particle_Mass *mass;
    float GRAVITY;
    float enviroment_density = 1;

public:
    glm::vec2 velocity = {0, 0};

    // terminal velocity je najvacsia mozna rychlost pre objekt sa hybat
    int terminal_velocity; // float nechcem lebo sa neviem pohnut dopice o 0.1 ci 0.9

public:
    // Constructs Particle_Velocity.
    Particle_Velocity(Particle_Density *density, Particle_Mass *mass, float GRAVITY);
    // Destroys Particle_Velocity and releases owned resources.
    ~Particle_Velocity() = default;

    // Calculates terminal velocity.
    void calculate_terminal_velocity();

    // Returns velocity.
    glm::vec2 get_velocity();
    // Returns velocity x.
    float get_velocity_x();
    // Returns velocity y.
    float get_velocity_y();

    // Sets environment density.
    void set_environment_density(float enviroment_density);
    // Sets velocity.
    void set_velocity(glm::vec2 velocity);
    // Sets velocity x.
    void set_velocity_x(float velocity_x);
    // Sets velocity y.
    void set_velocity_y(float velocity_y);

    // Changes velocity.
    void change_velocity(glm::vec2 velocity);
    // Changes velocity x.
    void change_velocity_x(float delta_velocity_x);
    // Changes velocity y.
    void change_velocity_y(float delta_velocity_y);
};
