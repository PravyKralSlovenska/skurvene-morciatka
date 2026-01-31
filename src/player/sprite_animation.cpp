#include "engine/player/sprite_animation.hpp"

#include "engine/player/entity_manager.hpp"
#include "engine/player/entity.hpp"
#include "engine/player/sprite_animation.hpp"

// stb_image included elsewhere (entities_renderer.cpp or herringbone_world_generation.cpp)
// DO NOT define STB_IMAGE_IMPLEMENTATION here - it causes multiple definition errors
#include "stb/stb_image.h"

void Sprite_Animation::update()
{
}

void Sprite_Animation::add_frame()
{
}

Animation_Frame Sprite_Animation::get_frame()
{
    return Animation_Frame();
}

void Sprite_Animation::play()
{
}
void Sprite_Animation::set_looping()
{
}

void Sprite_Animation::reset()
{
}

void Sprite_Animation::stop()
{
}