#include "engine/player/entity.hpp"

void Entity::set_hitbox_dimensions(const int width, const int height)
{
    set_hitbox_dimensions(glm::ivec2(width, height));
}

void Entity::set_hitbox_dimensions(const glm::ivec2 &hitbox_dimensions)
{
    this->hitbox_dimensions = hitbox_dimensions;

    hitbox_dimensions_half = this->hitbox_dimensions / 2;
}

void Entity::set_position(const int x, const int y)
{
    set_position(glm::ivec2(x, y));
}

void Entity::set_position(const glm::ivec2 &position)
{
    this->coords = position;
}

void Entity::calculate_hitbox()
{
    // pravy horny roh
    hitbox[0] = glm::ivec2(coords.x - hitbox_dimensions_half.x, coords.y - hitbox_dimensions_half.y);
    // pravy lavy roh
    hitbox[1] = glm::ivec2(coords.x + hitbox_dimensions_half.x, coords.y - hitbox_dimensions_half.y);
    // dolny pravy roh
    hitbox[2] = glm::ivec2(coords.x - hitbox_dimensions_half.x, coords.y + hitbox_dimensions_half.y);
    // dolny lavy roh
    hitbox[3] = glm::ivec2(coords.x + hitbox_dimensions_half.x, coords.y + hitbox_dimensions_half.y);
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
