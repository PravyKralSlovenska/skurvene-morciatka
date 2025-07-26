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
#include "engine/renderer/shader.hpp"
#include "engine/renderer/text_renderer.hpp"

#include "engine/camera.hpp"
#include "engine/controls.hpp"

struct Vertex
{
    float x, y;
    Color color;

    Vertex();
    Vertex(float x, float y, Color color);
    ~Vertex() = default;
};

class Renderer
{
private:
    float m_window_width, m_window_height;
    GLFWwindow *window;

    std::unique_ptr<Text_Renderer> text_renderer;
    // Camera camera;
        
    // cisto len na infosky
    std::vector<std::string> render_info;

public:
    bool running = true;

public:
    Renderer(float window_width, float window_height);
    ~Renderer() = default;

    void init();
    void init_glad();
    void init_glfw();

    void create_window();
    GLFWwindow *get_window();

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

    void render_triangle();
};