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
        // std::cout << x << ';' << y << '\n';

        world->add_particle({x, y}, Particle_Type::SAND, 3);
        // world->add_particle({x, y}, Particle_Type::WATER, 3);
        // world->add_particle({x, y}, Particle_Type::SMOKE, 3);
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
        world->clear_world();
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void Controls::left_mouse_click(double xpos, double ypos)
{
}