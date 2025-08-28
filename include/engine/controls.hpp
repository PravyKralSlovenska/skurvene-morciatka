#pragma once

#include <iostream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "engine/world/world.hpp"
#include "engine/entity.hpp"
#include "engine/particle/particle.hpp"

class Controls
{
private:
    GLFWwindow *window;
    Player *player;
    World *world;
    bool help_key_pressed = false;  // Track H key state for toggle

public:
    glm::vec2 cursor_position;
    bool show_help = false;  // Help display state
    Particle_Type current_particle = Particle_Type::SAND;  // Current particle type to spawn

public:
    Controls();
    ~Controls() = default;

    void set_player(Player *player);
    void set_window(GLFWwindow *window);
    void set_world(World *world);

    // inputs
    void handle_input();

    // keyboard controls
    void keyboard_input();
    void handle_inventory();

    void get_cursor_position();

    // mouse controls
    void left_mouse_click(double xpos, double ypos);
    void right_mouse_click(double xpos, double ypos);
    void handle_mouse_scroll(double xoffset, double yoffset);
    // void handle_mouse_move(double xpos, double ypos);
};