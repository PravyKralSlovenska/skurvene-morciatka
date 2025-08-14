#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <array>

#include <glm/glm.hpp>

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
class Particle
{
public:
    ParticleType type = ParticleType::EMPTY;   // konkretne co to je (Piesok)
    ParticleState state = ParticleState::NONE; // ake skupenstvo ma dana latka
    Color base_color = Color(0.0f, 0.0f, 0.0f, 0.0f);
    Color color = Color(0.0f, 0.0f, 0.0f, 0.0f);

public:
    Particle();
    Particle(ParticleType type, ParticleState state, Color base_color);
    ~Particle() = default;
};

Particle create_sand();
Particle create_water();
Particle create_smoke();

// teplota
// zivotnost
// velocity
// melting boiling point
// interactions
// struct Flags {
//     uint8_t is_moving : 1;
//     uint8_t needs_update : 1;
//     uint8_t is_static : 1;      // Never moves (optimization)
//     uint8_t is_reactive : 1;    // Can participate in chemical reactions
//     uint8_t reserved : 4;
// } flags = {0};

class Solid : public Particle
{
    virtual void move() = 0;
};