#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <array>

#include "others/utils.hpp"

enum class ParticleType
{
    EMPTY,
    SAND,
    WATER,
    SMOKE
};

enum class ParticleState
{
    NONE,
    SOLID,
    LIQUID,
    GAS
};


/*
 * Particle
 * - hlavna stavebna jednotka
 */
struct Particle
{
    ParticleType type   = ParticleType::EMPTY;      // konkretne co to je (Piesok)
    ParticleState state = ParticleState::NONE;      // ake skupenstvo ma dana latka
    Color color         = Color(0.0f, 0.0f, 0.0f, 0.0f);

    Particle();
    Particle(ParticleType type, ParticleState state, Color color);
    ~Particle() = default;
};

Particle create_sand();
Particle create_water();
Particle create_smoke();