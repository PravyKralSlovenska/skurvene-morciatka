#include "engine/audio/audio_source.hpp"
#include "engine/audio/audio_buffer.hpp"

Audio_Source::Audio_Source(glm::ivec3 coords, glm::ivec3 velocity, float pitch, float gain)
{
    alGenSources(1, &id);

    set_pitch(pitch);
    set_gain(gain);
    set_position(coords);
    set_velocity(velocity);
}

Audio_Source::Audio_Source()
{
    alGenSources(1, &id);
}

Audio_Source::~Audio_Source()
{
    alDeleteSources(1, &id);
}

void Audio_Source::play_sound(const Sound_Buffer &sound)
{
    // alSourceQueueBuffers(id, 1, &sound.id);
    alSourcei(id, AL_BUFFER, sound.id);

    alSourcePlay(id);
}

void Audio_Source::set_pitch(const float pitch)
{
    alSourcef(id, AL_PITCH, pitch);
}

void Audio_Source::set_gain(const float gain)
{
    alSourcef(id, AL_GAIN, gain);
}

void Audio_Source::set_position(const glm::ivec3 coords)
{
    alSource3i(id, AL_POSITION, coords.x, coords.y, coords.z);
}

void Audio_Source::set_velocity(const glm::ivec3 velocity)
{
    alSource3i(id, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Audio_Source::set_looping(const ALboolean yes)
{
    alSourcei(id, AL_LOOPING, yes);
}