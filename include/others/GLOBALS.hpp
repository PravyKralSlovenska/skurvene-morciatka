#pragma once

namespace Globals
{
    const float WINDOW_WIDTH = 1000.0;
    const float WINDOW_HEIGHT = 800.0;
    const float PARTICLE_SIZE = 10.0;

    const float CHUNK_WIDTH = 1000.0;
    const float CHUNK_HEIGHT = 800.0;
};

namespace Physics
{
    const float GRAVITY = 9.81f;              // m/s^2
    const float PI = 3.14159265358979323846f; // Pi constant
    const float AIR_RESISTANCE = 0.47f;       // Drag coefficient for a sphere
    const float AIR_DENSITY = 1.225f;         // kg/m^3 at sea level
    const float WATER_DENSITY = 1000.0f;      // kg/m^3 at 4 degrees Celsius
};