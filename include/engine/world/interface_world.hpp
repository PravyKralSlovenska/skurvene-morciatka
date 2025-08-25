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
    IWorld(float width, float height, float scale);
    ~IWorld() = default;

    void update();
    std::unique_ptr<World>& get_world();
};
