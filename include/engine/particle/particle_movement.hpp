#pragma once

#include <iostream>

#include "engine/particle/particle.hpp"

class Particle_Movement
{
    private:
        // susedia
    public: 
        void find_moore_neighborhood();
        static void move_solid(Particle *particle); // pohni sa dole, a dole do stran
        static void move_liquid(); // pohni sa dole, vedla seba a dole do stran
        static void move_gas(); /// pohni sa hore, vedla seba a hore do stran
};