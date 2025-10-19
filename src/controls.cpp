#include "engine/controls.hpp"

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    // TODO
    // if (-15.0 < zoom && zoom < 15.0)

    float offset = 0.1;
    if (yoffset < 0)
    {
        zoom = std::abs(zoom -= offset);
    }
    else
    {
        zoom += offset;
    }
}

Controls::Controls() {}

void Controls::set_player(Player *player)
{
    this->player = player;
}

void Controls::set_window(GLFWwindow *window)
{
    this->window = window;

    // pretoze je to set funkcia tak ju dam do konstruktora
    // keby bola niekde inde bolo by to furt ze nastavujem tu funkciu
    // tiez je to tu lebo window je sucast toho
    glfwSetScrollCallback(window, scroll_callback);
}

void Controls::set_world(World *world)
{
    this->world = world;
}

void Controls::set_time_manager(Time_Manager *time_manager)
{
    this->time_manager = time_manager;
}

void Controls::set_audio_manager(Audio_Manager *audio_manager)
{
    this->audio_manager = audio_manager;
}

void Controls::set_camera(Camera *camera)
{
    this->camera = camera;
}

void Controls::handle_input()
{
    if (!window || !player || !camera)
    {
        return;
    }

    camera->set_zoom(zoom);

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

    // if pressed the world update loop will stop
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        time_manager->pause();
    }

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        time_manager->resume();
    }

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        audio_manager->send_execute(Pending_Execute::PLAY, "background music");
    }

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        // audio_manager->send_execute(Pending_Execute::STOP, "background music");
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void Controls::left_mouse_click(double xpos, double ypos)
{
}
