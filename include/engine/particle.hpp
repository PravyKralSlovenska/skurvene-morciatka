#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <array>

enum class ParticleType
{
    SAND,
    WATER,
    SMOKE,
};

enum class ParticleState
{
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
    std::string name;
    ParticleType type;   // konkretne co to je (Piesok)
    ParticleState state; // ake skupenstvo ma dana latka

    int x; // x pozicia
    int y; // y pozicia

    int radius = 1; // radius, v ktorom vie ovplyvnovat

    float velocity_x;           // x rychlost
    float velocity_y;           // y rychlost
    float density;              // hustota
    float mass;                 // hmotnost
    int lifetime;               // zivotnost
    float temperature;          // teplota
    float ignition_temperature; // teplota pri ktorej zacne horiet
    float melting_point;        // teplota pri ktorej sa roztavi/topit sa
    float boiling_point;        // teplota pri ktorej sa vyprari

    std::array<int, 3> color; // farba v RGB formate (Red, Green, Blue)
    float opacity;              // alpha

    bool flammable;       // ci hori
    bool corrosive;       // ci je korozny
    bool conductive;      // ci vedie elektricky prud
    bool magnetic;        // ci je magneticky
    bool source_of_light; // ci vyzaruje svetlo
    bool radioactive;     // ci je radioaktivny
    bool explosive;       // ci moze explodovat
    bool toxic;           // ci je jedovaty

    Particle();
    Particle(
        std::string name,
        ParticleType type,
        ParticleState state,
        int x,
        int y,
        float vx,            // m/s
        float vy,            // m/s
        float density,       // kg/m^3
        float mass,          // kg
        int lifetime,        // s
        float temperature,   // Celzius
        float i_temperature, // Celzius
        float m_point,       // Celzius
        float b_point,       // Celzius
        bool flammable,
        bool corrosive,
        bool conductive,
        bool magnetic,
        bool source_of_light,
        bool radioactive,
        bool explosive,
        bool toxic,
        std::array<int, 3> color,
        float opacity);
    virtual ~Particle() = default;

    std::pair<int, int> getCoords();
    int get_x();
    int get_y();
    void add_x(int x);
    void set_x(int x);
    void add_y(int y);
    void set_y(int y);
    void set_hurt(bool h);
    void set_coords(int x, int y);
    void set_velocity(int vx = 0, int vy = 0);
    void set_temperature(int t);
    void set_color(std::array<int, 3> c);

    static Particle create_sand(int x, int y, float vx, float vy);
    static Particle create_water(int x, int y, float vx, float vy);
    static Particle create_smoke(int x, int y, float vx, float vy);
};
