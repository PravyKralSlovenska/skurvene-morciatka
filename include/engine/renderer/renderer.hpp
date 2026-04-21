#pragma once
// File purpose: Defines the main renderer setup and frame orchestration API.
#include <iostream>
#include <memory>
#include <functional>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "engine/renderer/renderer.hpp"
#include "engine/renderer/world_renderer.hpp"
#include "engine/renderer/text_renderer.hpp"
#include "engine/renderer/entities_renderer.hpp"
#include "engine/renderer/ui_renderer.hpp"

#include "engine/camera.hpp"
#include "engine/controls.hpp"
#include "engine/time_manager.hpp"
#include "engine/world/world.hpp"

// forward declaration
class Entity_Manager;

// Owns render context setup and frame orchestration.
class IRenderer
{
private:
    float m_window_width, m_window_height;
    GLFWwindow *window;
    bool is_fullscreen = false;
    int windowed_xpos, windowed_ypos;
    int windowed_width, windowed_height;

    // projection
    glm::mat4 projection;
    glm::mat4 view_projection;

    // rendere
    std::unique_ptr<World_Renderer> world_renderer;
    std::unique_ptr<Entities_Renderer> entities_renderer;
    std::unique_ptr<Text_Renderer> text_renderer;
    std::unique_ptr<UI_Renderer> ui_renderer;

    // srandy co potrebuhjem
    Camera *camera;
    Time_Manager *time_manager;
    World *world;
    Entity_Manager *entity_manager = nullptr;

    // cisto len na infosky
    std::vector<std::string> render_info;

public:
    // Constructs IRenderer.
    IRenderer(float window_width, float window_height);
    // Destroys IRenderer and releases owned resources.
    ~IRenderer() = default;

    // Initializes state.
    void init();
    // Initializes glad.
    void init_glad();
    // Initializes glfw.
    void init_glfw();

    // Creates window.
    void create_window();
    // Returns window.
    GLFWwindow *get_window();

    // will render everthing
    bool render_everything(bool render_world = true,
                           bool render_in_game_ui = true,
                           // No-op callback.
                           const std::function<void()> &overlay_ui = nullptr);

    // Sets time manager.
    void set_time_manager(Time_Manager *time_manager);
    // Sets world.
    void set_world(World *world);
    // Sets camera.
    void set_camera(Camera *camera);
    // Sets entity manager.
    void set_entity_manager(Entity_Manager *entity_manager);

    // Updates camera uniforms.
    void update_camera_uniforms();

    // Enables blending.
    void enable_blending();
    // Enables ortho projection.
    void enable_ortho_projection();
    // Enables pixel perfect rendering.
    void enable_pixel_perfect_rendering();
    // Updates projection on resize.
    void update_projection_on_resize();

    // Returns true if close.
    bool should_close();

    // Toggles fullscreen.
    void toggle_fullscreen();
    // Maximizes window.
    void maximize_window();
    // Returns fullscreen state.
    bool get_fullscreen_state();

    // Cleanups this component state.
    void cleanup();

    // ImGui
    void init_imgui();
    // Cleans up imgui.
    void cleanup_imgui();
    // Imgui new frame.
    void imgui_new_frame();
    // Imgui render.
    void imgui_render();

    // UI
    void toggle_debug_info();
    // Toggles fullscreen map.
    void toggle_fullscreen_map();
    // Returns true if fullscreen map open.
    bool is_fullscreen_map_open() const;
    // Sets loading screen state.
    void set_loading_screen_state(float progress, const std::string &status);
    Menu_Actions render_menu_screen(Menu_Screen screen,
                                    bool enter_pressed,
                                    bool escape_pressed,
                                    Menu_Options_Model &options);

    // debug
    void print_render_info();
    // Checks glerror.
    void checkGLError(const char *operation);
};
