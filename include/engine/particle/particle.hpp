#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <array>

#include <glm/glm.hpp>

#include "others/utils.hpp"

// ake skupenstvo ma dana latka
enum class Particle_State
{
    NONE,
    SOLID,
    LIQUID,
    GAS,
    PLASMA
};

// konkretne co to je (Piesok, ...)
enum class Particle_Type
{
    EMPTY,
    SAND,
    WATER,
    SMOKE,
    STONE
};

enum class Particle_Movement : uint8_t
{
    NONE = 0,
    DOWN = 1 << 0,
    UP = 1 << 1,
    RIGHT = 1 << 2,
    LEFT = 1 << 3,
    DOWN_RIGHT = 1 << 4,
    DOWN_LEFT = 1 << 5,
    UP_RIGHT = 1 << 6,
    UP_LEFT = 1 << 7,

    MOVE_SIDES = LEFT | RIGHT,
    MOVE_SOLID = DOWN | DOWN_LEFT | DOWN_RIGHT,
    MOVE_LIQUID = DOWN | DOWN_LEFT | DOWN_RIGHT | MOVE_SIDES,
    MOVE_GAS = UP | UP_LEFT | UP_RIGHT | MOVE_SIDES
};

inline Particle_Movement operator|(Particle_Movement a, Particle_Movement b)
{
    return Particle_Movement((int)a | (int)b);
}

inline auto operator&(Particle_Movement a, Particle_Movement b)
{
    return (int)a & (int)b;
}

/*
 * Particle
 * - hlavna stavebna jednotka
 */
class Particle
{
public:
    Particle_Type type = Particle_Type::EMPTY;
    Particle_State state = Particle_State::NONE;
    Particle_Movement move = Particle_Movement::NONE;

    Color base_color = Color(0, 0, 0, 0.0f);
    Color color = Color(0, 0, 0, 0.0f);

public:
    Particle();
    Particle(Particle_Type type, Particle_State state, Particle_Movement move, Color base_color);
    ~Particle() = default;
};

Particle create_sand();
Particle create_water();
Particle create_smoke();
Particle create_stone();

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
