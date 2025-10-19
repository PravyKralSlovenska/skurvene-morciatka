#pragma once

#include <glm/glm.hpp>
#include <AL/al.h>

class Sound_Buffer;

/*
 * Sources
 * - zdroj, z ktore "vznika" zvuk
 * - "reproduktor"
 */
class Audio_Source
{
public:
    ALuint id;
    glm::vec3 coords; // suradnice x,y,z kedze sme v 2D, suradnica z bude 1 alebo 0
    // std::unique_ptr<Sound_Buffer> sound;

    bool currently_playing;

public:
    Audio_Source(glm::ivec3 coords, glm::ivec3 velocity = {0, 0, 0}, float pitch = 1.0f, float gain = 1.0f);
    Audio_Source();
    ~Audio_Source();

    void play_sound(const Sound_Buffer &sound);
    void stop_sound();
    void pause_sound();

    void set_pitch(const float pitch);
    void set_gain(const float gain);
    void set_position(const glm::ivec3 position);
    void set_velocity(const glm::ivec3 velocity);
    void set_looping(const ALboolean yes);
};
