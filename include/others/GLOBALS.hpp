#pragma once

namespace Globals
{
    // constexpr float WINDOW_WIDTH = 1920.0;
    // constexpr float WINDOW_HEIGHT = 1080.0;
    constexpr float WINDOW_WIDTH = 1000.0;
    constexpr float WINDOW_HEIGHT = 800.0;
    constexpr float PARTICLE_SIZE = 2.0; // particle size 1 == seka idk preco

    constexpr float CHUNK_WIDTH = 1000.0;
    constexpr float CHUNK_HEIGHT = 800.0;
};

// const - klasicka konstanta
// constexpr - konstanta, ktora musi byt znama uz pocas kompilovania

namespace Physics
{
    constexpr int ONE_SIDE_OF_A_PARTICLE = 1;     // m
    constexpr float GRAVITY = 9.81f;              // m/s^2
    constexpr float PI = 3.14159265358979323846f; // Pi constant
    constexpr float AIR_RESISTANCE = 0.47f;       // Drag coefficient for a sphere
    constexpr float AIR_DENSITY = 1.225f;         // kg/m^3 at sea level
    constexpr float WATER_DENSITY = 1000.0f;      // kg/m^3 at 4 degrees Celsius
};
