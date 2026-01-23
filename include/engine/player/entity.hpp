#pragma once

#include <iostream>
#include <array>
#include <glm/glm.hpp>

// forward declarations
class Item;

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
    int sprite; // konkretne ktory snimok je aktualny
    float speed = 10;
    float healthpoints;
    glm::vec2 velocity;

    glm::ivec2 coords;                 // stred hitboxu, sledujeho kamera
    glm::ivec2 hitbox_dimensions;      // sirka a vyska
    glm::ivec2 hitbox_dimensions_half; //
    glm::ivec2 hitbox[4];              // konretne 4 body vo svete, ktore tvoria hranice hitboxu

    std::string entity_sprite; // kazdy entity sprite bude rovnako velky

public:
    // setters
    void set_hitbox_dimensions(const int width, const int height);
    void set_hitbox_dimensions(const glm::ivec2 &hitbox_dimensions);

    void set_position(const int x, const int y);
    void set_position(const glm::ivec2 &position);

    // nemozem to dat private
    void calculate_hitbox();

    // getters

    // actions
    void select_item();
    void shoot();
    void heal();
    // void do_sum();

    // moving
    void move_up();
    void move_down();
    void move_left();
    void move_right();
    void jump();
    void move_by(float dx, float dy);
};

class Player : public Entity
{
private:
    std::array<Item, 50> inventory;

public:
    std::string name;
    int selected_item;

public:
    Player(std::string name, glm::vec2 coords);
    ~Player() = default;

    void change_selected_item(const int inventory_slot);
    void change_selected_item(const Item item);
};
