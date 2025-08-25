#include "engine/entity.hpp"

void Entity::set_position(float x, float y)
{
    coords = glm::vec2(x, y);
}

void Entity::move_up()
{
    coords.y -= speed;
}

void Entity::move_down()
{
    coords.y += speed;
}

void Entity::move_left()
{
    coords.x -= speed;
}

void Entity::move_right()
{
    coords.x += speed;
}

void Entity::move_by(float dx, float dy)
{
    coords.x += dx;
    coords.y += dy;
}

Player::Player(std::string name, glm::vec2 coords)
    : name(name)
{
    this->coords = coords;
}
