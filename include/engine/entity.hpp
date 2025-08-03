#pragma once

#include <iostream>
#include <glm/glm.hpp>

enum Entity_States
{
    STILL,
    WALKING,
    JUMPING,
    FALLING,
    HIT,
    DEAD
};

class Entity
{
public:
    int ID;
    Entity_States state;
    int size;
    int sprite;
    float speed;
    float healthpoints;
    glm::vec2 coords;

public:
    // moving
    void set_position(float x, float y);
    void move_up();
    void move_down();
    void move_left();
    void move_right();
    void jump();
    void move_by(float dx, float dy);
};

class Player : public Entity
{
public:
    std::string name;
    int selected_item;

public:
    Player(std::string name, glm::vec2 coords);
    ~Player() = default;

    void select_item();
    void shoot();
};