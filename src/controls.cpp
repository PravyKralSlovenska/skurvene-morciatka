#include "engine/controls.hpp"

Controls::Controls() {}

void Controls::set_player(Player *player)
{
    this->player = player;
}

void Controls::set_window(GLFWwindow *window)
{
    this->window = window;
}

void Controls::set_world(World *world)
{
    this->world = world;
}

void Controls::handle_input()
{
    if (!window || !player)
    {
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    cursor_position = {xpos, ypos};

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        int x = (int)xpos / world->scale;
        int y = (int)ypos / world->scale;

        world->add_particle({x, y}, Particle_Type::STONE, 3);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        int x = (int)xpos / world->scale;
        int y = (int)ypos / world->scale;

        world->add_particle({x, y}, selected_particle, 3);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_4) == GLFW_PRESS)
    {
        int x = (int)xpos / world->scale;
        int y = (int)ypos / world->scale;


        world->add_particle({x, y}, Particle_Type::EMPTY, 3);
    }

    keyboard_input();
}

void Controls::keyboard_input()
{
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

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        world->clear_world_curr();
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        selected_particle = Particle_Type::SAND;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        selected_particle = Particle_Type::WATER;
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        selected_particle = Particle_Type::SMOKE;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void Controls::left_mouse_click(double xpos, double ypos)
{
}