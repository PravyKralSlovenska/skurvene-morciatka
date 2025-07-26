#include <iostream>

#include "engine/entity.hpp"

void Entity::set_position(float x, float y)
{
    coords = glm::vec2(x, y);
}

void Entity::move_up()
{
    coords.y -= 1.0f;
}

void Entity::move_down()
{
    coords.y += 1.0f;
}

void Entity::move_left()
{
    coords.x -= 1.0f;
}

void Entity::move_right()
{
    std::cout << "Moving right" << std::endl;
    coords.x += 1.0f;
}

void Entity::move_by(float dx, float dy)
{
    coords.x += dx;
    coords.y += dy;
}