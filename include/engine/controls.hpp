#pragma once

#include <iostream>
#include <algorithm>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "engine/world/world.hpp"
#include "engine/time_manager.hpp"
#include "engine/entity.hpp"
#include "engine/camera.hpp"
#include "engine/audio/audio_manager.hpp"

static float zoom = 1.0;
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
// void click_callback() // ???

class Controls
{
private:
    GLFWwindow *window;
    Player *player;
    World *world;
    Time_Manager *time_manager;
    Audio_Manager *audio_manager;
    Camera *camera;

public:
    glm::vec2 cursor_position;
    Particle_Type selected_particle = Particle_Type::EMPTY;

public:
    Controls();
    ~Controls() = default;

    void set_player(Player *player);
    void set_window(GLFWwindow *window);
    void set_world(World *world);
    void set_time_manager(Time_Manager *time_manager);
    void set_audio_manager(Audio_Manager *audio_manager);
    void set_camera(Camera *camera);

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