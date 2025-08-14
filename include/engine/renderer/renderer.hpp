#pragma once
#include <iostream>
#include <memory>
// #include <sstream>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/renderer.hpp"
#include "engine/renderer/world_renderer.hpp"
#include "engine/renderer/text_renderer.hpp"
#include "engine/renderer/entities_renderer.hpp"

#include "engine/camera.hpp"
#include "engine/controls.hpp"
#include "engine/world/world.hpp"

class IRenderer
{
private:
    float m_window_width, m_window_height;
    float scale;
    GLFWwindow *window;

    // projection
    glm::mat4 projection;
    glm::mat4 view_projection;

    // rendere
    std::unique_ptr<World_Renderer> world_renderer;
    std::unique_ptr<Entities_Renderer> entities_renderer;
    std::unique_ptr<Text_Renderer> text_renderer;

    // svet
    World *world = nullptr;
    // cisto len na infosky
    std::vector<std::string> render_info;

    // FPS
    double start_time = glfwGetTime();
    double previous_time = glfwGetTime();
    int frame_count = 0;
    int frame_count_display = 0;

public:
    IRenderer(float window_width, float window_height, float scale, World *world);
    ~IRenderer() = default;

    void init();
    void init_glad();
    void init_glfw();

    void create_window();
    GLFWwindow *get_window();

    void set_world(World *world);

    void set_camera(Camera *camera);
    void update_camera_uniforms();

    void enable_blending();
    void enable_ortho_projection();
    void enable_pixel_perfect_rendering();

    bool should_close();

    void cleanup();

    // debug
    void print_render_info();
    void checkGLError(const char *operation);

    // will render everthing
    bool render_everything();
};
