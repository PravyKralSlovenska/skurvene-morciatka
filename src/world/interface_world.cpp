#include "engine/world/interface_world.hpp"

IWorld::IWorld(float width, float height, float scale)
{
    world = std::make_unique<World>(width, height, scale);
}

void IWorld::update()
{
    world->update_world_loop();
}

std::unique_ptr<World>& IWorld::get_world()
{
    return world;
}