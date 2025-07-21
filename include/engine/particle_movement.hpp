#pragma once

#include <iostream>

class Particle_movement
{
    private:
        // susedia
    public: 
        void find_moore_neighborhood();
        void move_solid(); // pohni sa dole, a dole do stran
        void move_liquid(); // pohni sa dole, vedla seba a dole do stran
        void move_gas(); /// pohni sa hore, vedla seba a hore do stran
};