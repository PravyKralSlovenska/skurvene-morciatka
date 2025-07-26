#include <iostream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "engine/controls.hpp"

Controls::Controls(Player *player)
    : player(player) {}

void Controls::set_window(GLFWwindow *window)
{
    this->window = window;
}

void Controls::handle_input()
{
    if (!window || !player)
    {
        return;
    }

    keyboard_input();
}

void Controls::keyboard_input()
{
    if (!window || !player)
    {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        player->move_up();
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        player->move_down();
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        player->move_left();
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        player->move_right();
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

