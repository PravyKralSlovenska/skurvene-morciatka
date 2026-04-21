#pragma once

// File purpose: Implements movement rules for particles in the world grid.
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

#include "engine/particle/particle.hpp"

class World;
class Chunk;
class WorldCell;

// Applies movement rules for particle updates.
class Particle_Movement_Engine
{
private:
    // susedia
    std::unordered_map<std::string, std::string> neighbors_map = {
        {"up-left", "up-left"}, {"up", "up"}, {"up-right", "up-right"}, {"next-left", "next-left"}, /*center*/ {"next-right", "next-right"}, {"down-left", "down-left"}, {"down", "down"}, {"down-right", "down-right"}};

    // Tries move particle.
    bool try_move_particle(World *world, const glm::ivec2 &chunk_coords, int x, int y, const glm::ivec2 &direction);
    // Returns cell at.
    WorldCell *get_cell_at(World *world, const glm::ivec2 &chunk_coords, int x, int y);

public:
    // Finds places.
    std::map<std::string, Particle &> find_places(World *world, Particle *particle);                                                                 // should return array
    // Finds moore neighborhood.
    std::map<std::string, Particle &> find_moore_neighborhood(World *world, Particle *particle, const std::vector<glm::ivec2> &neighborhood_coords); // should return array

    // Moves solid.
    void move_solid(World *world, const glm::ivec2 &chunk_coords, int x, int y);  // pohni sa dole, a dole do stran
    // Moves liquid.
    void move_liquid(World *world, const glm::ivec2 &chunk_coords, int x, int y); // pohni sa dole, vedla seba a dole do stran
    // Moves gas.
    void move_gas(World *world, const glm::ivec2 &chunk_coords, int x, int y);    // pohni sa hore, vedla seba a hore do stran

    // Moves.
    void move(Particle *this_particle, Particle *other_particle);
};
