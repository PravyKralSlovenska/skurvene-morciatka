#pragma once

#include <GLFW/glfw3.h>
#include "engine/entity.hpp"

class Controls
{
private:
    GLFWwindow *window;
    Player *player;

public:
    glm::vec2 cursor_position;

public:
    Controls(Player *player);
    ~Controls() = default;

    void set_player(Player *player);
    void set_window(GLFWwindow *window);

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