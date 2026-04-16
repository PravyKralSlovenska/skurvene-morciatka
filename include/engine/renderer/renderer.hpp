#pragma once
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
    IRenderer(float window_width, float window_height);
    ~IRenderer() = default;

    void init();
    void init_glad();
    void init_glfw();

    void create_window();
    GLFWwindow *get_window();

    // will render everthing
    bool render_everything(bool render_world = true,
                           bool render_in_game_ui = true,
                           const std::function<void()> &overlay_ui = nullptr);

    void set_time_manager(Time_Manager *time_manager);
    void set_world(World *world);
    void set_camera(Camera *camera);
    void set_entity_manager(Entity_Manager *entity_manager);

    void update_camera_uniforms();

    void enable_blending();
    void enable_ortho_projection();
    void enable_pixel_perfect_rendering();
    void update_projection_on_resize();

    bool should_close();

    void toggle_fullscreen();
    void maximize_window();
    bool get_fullscreen_state();

    void cleanup();

    // ImGui
    void init_imgui();
    void cleanup_imgui();
    void imgui_new_frame();
    void imgui_render();

    // UI
    void toggle_debug_info();
    void toggle_fullscreen_map();
    bool is_fullscreen_map_open() const;
    void set_loading_screen_state(float progress, const std::string &status);
    Menu_Actions render_menu_screen(Menu_Screen screen,
                                    bool enter_pressed,
                                    bool escape_pressed,
                                    Menu_Options_Model &options);

    // debug
    void print_render_info();
    void checkGLError(const char *operation);
};
