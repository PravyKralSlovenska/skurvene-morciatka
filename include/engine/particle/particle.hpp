#pragma once

#include <memory>
#include <vector>
#include <array>
#include <cstdint>

#include <glm/glm.hpp>

#include "others/GLOBALS.hpp"
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
    URANIUM,
    STONE
};

enum class Particle_Movement : uint8_t
{
    STATIC = 0,
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
 * Particle_Physics
 * - fyzikalne vlastnosti castice
 * - pouziva sa na falling sand simulaciu a chemicke reakcie
 */
struct Particle_Physics
{
    // Density (hustota) - kg/m^3
    // Sand: ~1600 kg/m^3, Water: ~1000 kg/m^3
    float density = 0.0f;

    // Temperature (teplota) - Kelvin
    // Room temperature: ~293K (20°C)
    float temperature = 293.0f;

    // Velocity (rychlost) - cells per update
    glm::vec2 velocity = {0.0f, 0.0f};

    // Acceleration accumulator for physics
    glm::vec2 acceleration = {0.0f, 0.0f};

    // Melting point (bod topenia) - Kelvin
    float melting_point = 0.0f;

    // Boiling point (bod varu) - Kelvin
    float boiling_point = 0.0f;

    // Thermal conductivity (tepelna vodivost) - how fast heat transfers
    float thermal_conductivity = 0.5f;

    // Specific heat capacity - how much energy to change temperature
    float specific_heat = 1.0f;

    // Viscosity (viskozita) - resistance to flow (higher = slower spreading)
    // Water: ~0.001, Honey: ~2.0
    float viscosity = 0.0f;

    // Dispersion rate - how much liquid spreads horizontally
    int dispersion_rate = 1;

    Particle_Physics() = default;
    Particle_Physics(float density, float temperature, float melting_point, float boiling_point);
};

/*
 * Particle_Flags
 * - bitove flagy pre optimalizaciu a stav castice
 */
struct Particle_Flags
{
    uint8_t is_static : 1;    // World-generated, won't move (terrain)
    uint8_t is_updated : 1;   // Already processed this frame
    uint8_t is_falling : 1;   // Currently falling
    uint8_t is_reactive : 1;  // Can participate in chemical reactions
    uint8_t is_flammable : 1; // Can catch fire
    uint8_t is_on_fire : 1;   // Currently burning
    uint8_t reserved : 2;

    Particle_Flags() : is_static(0), is_updated(0), is_falling(0),
                       is_reactive(0), is_flammable(0), is_on_fire(0), reserved(0) {}
};

/*
 * Particle
 * - hlavna stavebna jednotka
 */
class Particle
{
public:
    Particle_Type type = Particle_Type::EMPTY;
    Particle_State state = Particle_State::NONE;
    Particle_Movement move = Particle_Movement::STATIC;

    Color base_color = Color(0, 0, 0, 0.0f);
    Color color = Color(0, 0, 0, 0.0f);

    // Physics properties
    Particle_Physics physics;

    // Optimization flags
    Particle_Flags flags;

    // Lifetime in ticks (0 = infinite)
    uint16_t lifetime = 0;

public:
    bool operator==(const Particle &other) const
    {
        return type == other.type && state == other.state;
    }

    // Check if particle can move (not static and not empty)
    bool can_move() const
    {
        return !flags.is_static && type != Particle_Type::EMPTY && move != Particle_Movement::STATIC;
    }

    // Check if this particle is heavier than another (for displacement)
    bool is_heavier_than(const Particle &other) const
    {
        return physics.density > other.physics.density;
    }

    // Check if particle can displace another
    bool can_displace(const Particle &other) const
    {
        if (other.type == Particle_Type::EMPTY)
            return true;
        if (other.flags.is_static)
            return false;
        // Heavier particles can displace lighter ones (sand sinks in water)
        return is_heavier_than(other) && other.state == Particle_State::LIQUID;
    }

    // Reset update flag for new frame
    void reset_update_flag()
    {
        flags.is_updated = 0;
    }

    // Mark as updated this frame
    void mark_updated()
    {
        flags.is_updated = 1;
    }

    // Set as static (for world generation)
    void set_static(bool value = true)
    {
        flags.is_static = value ? 1 : 0;
    }

    Particle();
    Particle(Particle_Type type, Particle_State state, Particle_Movement move, Color base_color);
    Particle(Particle_Type type, Particle_State state, Particle_Movement move, Color base_color, Particle_Physics physics);
    ~Particle() = default;
};

// Factory functions for creating particles
Particle create_sand(bool is_static = false);
Particle create_water(bool is_static = false);
Particle create_smoke(bool is_static = false);
Particle create_stone(bool is_static = false);
Particle create_uranium(bool is_static = false);

// Create empty particle
Particle create_empty();
