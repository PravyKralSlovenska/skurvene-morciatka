#pragma once

// File purpose: Maps keyboard and mouse input to game actions.
#include "engine/particle/particle.hpp"

// forward declarations
class Player;
class World;
class Time_Manager;
class Audio_Manager;
class Camera;
class IRenderer;
class Entity_Manager;
class GLFWwindow;

static float zoom = 1.0;
// Scroll callback.
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
// Maps input events to gameplay actions.

class Controls
{
private:
    GLFWwindow *window = nullptr;
    Player *player = nullptr;
    World *world = nullptr;
    Time_Manager *time_manager = nullptr;
    Audio_Manager *audio_manager = nullptr;
    Camera *camera = nullptr;
    IRenderer *renderer = nullptr;
    Entity_Manager *entity_manager = nullptr;

public:
    glm::vec2 cursor_position;
    Particle_Type selected_particle = Particle_Type::EMPTY;

public:
    // Constructs Controls.
    Controls();
    // Destroys Controls and releases owned resources.
    ~Controls() = default;

    // Sets player.
    void set_player(Player *player);
    // Sets window.
    void set_window(GLFWwindow *window);
    // Sets world.
    void set_world(World *world);
    // Sets time manager.
    void set_time_manager(Time_Manager *time_manager);
    // Sets audio manager.
    void set_audio_manager(Audio_Manager *audio_manager);
    // Sets camera.
    void set_camera(Camera *camera);
    // Sets renderer.
    void set_renderer(IRenderer *renderer);
    // Sets entity manager.
    void set_entity_manager(Entity_Manager *entity_manager);

    // inputs
    void handle_input();

    // keyboard controls
    void keyboard_input();
    // void handle_inventory();
    void get_cursor_position();

    // mouse controls
    void left_mouse_click(double xpos, double ypos);
    // Right mouse click.
    void right_mouse_click(double xpos, double ypos);
    // Handle mouse scroll.
    void handle_mouse_scroll(double xoffset, double yoffset);
    // void handle_mouse_move(double xpos, double ypos);
};
