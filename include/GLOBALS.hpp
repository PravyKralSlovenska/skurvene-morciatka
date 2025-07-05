#pragma once
#include <iostream>

namespace Globals
{
    const int WINDOW_HEIGHT = 800;
    const int WINDOW_WIDTH = 500;
    const int PARTICLE_SIZE = 50;
    const int WORLD_RIADKY = WINDOW_WIDTH / PARTICLE_SIZE - 1;
    const int WORLD_STLPCE = WINDOW_HEIGHT / PARTICLE_SIZE - 1;
    // chunky
};

namespace Physics
{
    const float GRAVITY = 9.81f; // m/s^2
    const float PI = 3.14159265358979323846f; // Pi constant
    const float AIR_RESISTANCE = 0.47f; // Drag coefficient for a sphere
    const float AIR_DENSITY = 1.225f; // kg/m^3 at sea level
    const float WATER_DENSITY = 1000.0f; // kg/m^3 at 4 degrees Celsius
};