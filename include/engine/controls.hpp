#pragma once

#include "engine/particle/particle.hpp"

// forward declarations
class Player;
class World;
class Time_Manager;
class Audio_Manager;
class Camera;
class IRenderer;
class GLFWwindow;

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
    IRenderer *renderer;

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
    void set_renderer(IRenderer *renderer);

    // inputs
    void handle_input();

    // keyboard controls
    void keyboard_input();
    // void handle_inventory();
    void get_cursor_position();

    // mouse controls
    void left_mouse_click(double xpos, double ypos);
    void right_mouse_click(double xpos, double ypos);
    void handle_mouse_scroll(double xoffset, double yoffset);
    // void handle_mouse_move(double xpos, double ypos);
};