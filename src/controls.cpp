#include "engine/controls.hpp"

#include <iostream>
#include <algorithm>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "engine/world/world.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/time_manager.hpp"
#include "engine/player/entity.hpp"
#include "engine/player/wand.hpp"
#include "engine/camera.hpp"
#include "engine/audio/audio_manager.hpp"

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

void Controls::set_renderer(class IRenderer *renderer)
{
    this->renderer = renderer;
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

    // Convert screen cursor to world coordinates
    glm::vec2 cam_pos = camera->get_position();
    float zoom_factor = camera->get_zoom();
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    glm::vec2 cursor_world;
    cursor_world.x = (xpos - window_width / 2.0f) / zoom_factor + cam_pos.x;
    cursor_world.y = (ypos - window_height / 2.0f) / zoom_factor + cam_pos.y;

    // Update player's cursor position for wand aiming
    player->set_cursor_world_pos(cursor_world);

    // Calculate aim direction
    glm::vec2 player_center = player->get_center();
    glm::vec2 aim_dir = cursor_world - player_center;
    if (glm::length(aim_dir) > 0.001f)
    {
        player->set_aim_direction(aim_dir);
    }

    // Handle wand usage with left mouse button
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        Wand &wand = player->get_current_wand();

        if (!wand.is_empty())
        {
            glm::ivec2 target_pos = glm::ivec2(cursor_world);
            int brush = wand.brush_size;

            // Place particles in a square brush area
            for (int dx = -brush / 2; dx <= brush / 2; dx++)
            {
                for (int dy = -brush / 2; dy <= brush / 2; dy++)
                {
                    glm::ivec2 pos = target_pos + glm::ivec2(dx, dy);

                    if (wand.type == Wand_Type::DELETE_WAND)
                    {
                        // Delete wand removes any particle
                        world->place_particle(pos, Particle_Type::EMPTY);
                    }
                    else
                    {
                        // Other wands place their particle type
                        world->place_particle(pos, wand.particle_type);
                    }
                }
            }
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        // Right click could be secondary action (e.g., larger brush, different mode)
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_4) == GLFW_PRESS)
    {
        // int x = (int)xpos / world->scale;
        // int y = (int)ypos / world->scale;

        // world->add_particle({x, y}, Particle_Type::EMPTY, 3);

        // std::cout << "CLICK STREDNE TLACIDLO\n";
    }

    keyboard_input();
}

void Controls::keyboard_input()
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        // std::cout << "W\n";
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
        // world->clear_world_curr();
    }

    // Hotbar selection (1-9 keys)
    static bool number_key_pressed[9] = {false};

    for (int i = 0; i < 9; i++)
    {
        int key = GLFW_KEY_1 + i; // GLFW_KEY_1, GLFW_KEY_2, ... GLFW_KEY_9
        if (glfwGetKey(window, key) == GLFW_PRESS)
        {
            if (!number_key_pressed[i])
            {
                player->select_hotbar_slot(i);
                Wand &wand = player->get_current_wand();
                std::cout << "Selected slot " << (i + 1) << ": " << wand.name << std::endl;
                number_key_pressed[i] = true;
            }
        }
        else
        {
            number_key_pressed[i] = false;
        }
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

    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
    {
        if (renderer)
        {
            renderer->toggle_fullscreen();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS)
    {
        if (renderer)
        {
            renderer->maximize_window();
        }
    }

    // TAB - Toggle noclip mode (allows player to move through solid terrain)
    static bool tab_pressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        if (!tab_pressed)
        {
            player->toggle_noclip();
            tab_pressed = true;
            std::cout << "Noclip: " << (player->get_noclip() ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        tab_pressed = false;
    }
}

void Controls::left_mouse_click(double xpos, double ypos)
{
}
