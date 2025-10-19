#include "engine/audio/listener.hpp"
#include <AL/al.h>

Listener::Listener()
{
}

void Listener::set_gain(const float gain)
{
    alListenerf(AL_GAIN, gain);
}

void Listener::set_position(const glm::ivec3 coords)
{
    alListener3i(AL_POSITION, coords.x, coords.y, coords.z);
}

void Listener::set_velocity(const glm::ivec3 velocity)
{
    alListener3i(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Listener::set_orientation(const glm::ivec3 orientation)
{
    // default je {0, 0, -1, 0, 1 , 0} neviem co to znamena ale okej
    alListener3f(AL_ORIENTATION, orientation.x, orientation.y, orientation.z);
}
