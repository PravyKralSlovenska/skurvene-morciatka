#pragma once

#include <iostream>
#include <memory>

#include "engine/world/world.hpp"
#include "engine/particle/particle.hpp"

class IWorld
{
private:
    std::unique_ptr<World> world;

public:
    IWorld(float width, float hight, float scale);
    ~IWorld() = default;
};
