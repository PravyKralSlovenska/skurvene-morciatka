#pragma once

#include <iostream>
#include <map>

#include "engine/particle/particle.hpp"
#include "engine/world/world.hpp"
#include "others/utils.hpp"

class Particle_Movement
{
private:
    // susedia
    std::map<glm::ivec2, std::string> neighbors_map = {
        {{1, 1}, "up-left"}, {{0, 1}, "up"}, {{-1, 1}, "up-right"}, {{1, 0}, "next-left"}, /*center*/ {{-1, 0}, "next-right"}, {{1, -1}, "down-left"}, {{0, -1}, "down"}, {{-1, -1}, "down-right"}};

public:
    std::map<std::string, Particle &> find_places(World *world, Particle *particle);                                                                          // should return array
    std::map<std::string, Particle &> find_moore_neighborhood(World *world, Particle *particle, const std::vector<glm::ivec2> &neighborhood_coords); // should return array

    void move_solid(World *world, Particle *particle);  // pohni sa dole, a dole do stran
    void move_liquid(World *world, Particle *particle); // pohni sa dole, vedla seba a dole do stran
    void move_gas(World *world, Particle *particle);    // pohni sa hore, vedla seba a hore do stran

    void move(Particle *this_particle, Particle *other_particle);
};