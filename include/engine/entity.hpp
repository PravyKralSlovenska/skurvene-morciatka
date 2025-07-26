#pragma once

#include <iostream>
#include <glm/glm.hpp>

class Entity
{
public:
    int ID;
    float healthpoints;
    glm::vec2 coords;

public:
    // moving
    void set_position(float x, float y);
    void move_up();
    void move_down();
    void move_left();
    void move_right();
    void move_by(float dx, float dy);
};

class Player : public Entity
{
public:
    int selected_item;
public:
    void select_item();
    void shoot();
};