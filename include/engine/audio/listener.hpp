#pragma once

#include <glm/glm.hpp>

/*
 * Listener
 * - clovek/hrac, ku ktoremu zvuk potuje
 */
class Listener
{
public:
    glm::vec2 coords;
    glm::vec2 velocity;

public:
    Listener();
    ~Listener() = default;

    void set_gain(const float gain);
    void set_position(const glm::ivec3 coords);
    void set_velocity(const glm::ivec3 velocity);
    void set_orientation(const glm::ivec3 orientation);
};
