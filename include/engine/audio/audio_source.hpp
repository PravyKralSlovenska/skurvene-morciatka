#pragma once

// File purpose: Wraps an OpenAL source used to play one sound stream.
#include <glm/glm.hpp>
#include <AL/al.h>

class Sound_Buffer;

/*
 * Sources
 * - zdroj, z ktore "vznika" zvuk
 * - "reproduktor"
 */
// Controls one OpenAL source instance.
class Audio_Source
{
public:
    ALuint id;
    glm::vec3 coords; // suradnice x,y,z kedze sme v 2D, suradnica z bude 1 alebo 0
    // std::unique_ptr<Sound_Buffer> sound;

    bool currently_playing;

public:
    // Constructs Audio_Source.
    Audio_Source(glm::ivec3 coords, glm::ivec3 velocity = {0, 0, 0}, float pitch = 1.0f, float gain = 1.0f);
    // Constructs Audio_Source.
    Audio_Source();
    // Destroys Audio_Source and releases owned resources.
    ~Audio_Source();

    // Plays sound.
    void play_sound(const Sound_Buffer &sound);
    // Stops sound.
    void stop_sound();
    // Pauses sound.
    void pause_sound();

    // Sets pitch.
    void set_pitch(const float pitch);
    // Sets gain.
    void set_gain(const float gain);
    // Sets position.
    void set_position(const glm::ivec3 position);
    // Sets velocity.
    void set_velocity(const glm::ivec3 velocity);
    // Sets looping.
    void set_looping(const ALboolean yes);
};
