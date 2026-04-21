#pragma once

// File purpose: Defines listener position/orientation controls for 3D audio.
#include <glm/glm.hpp>

/*
 * Listener
 * - clovek/hrac, ku ktoremu zvuk potuje
 */
// Represents global OpenAL listener state.
class Listener
{
public:
    glm::vec2 coords;
    glm::vec2 velocity;

public:
    // Constructs Listener.
    Listener();
    // Destroys Listener and releases owned resources.
    ~Listener() = default;

    // Sets gain.
    void set_gain(const float gain);
    // Sets position.
    void set_position(const glm::ivec3 coords);
    // Sets velocity.
    void set_velocity(const glm::ivec3 velocity);
    // Sets orientation.
    void set_orientation(const glm::ivec3 orientation);
};
