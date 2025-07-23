#pragma once
#include <iostream>
#include <vector>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "others/utils.hpp"
#include "engine/camera.hpp"
#include "engine/renderer/text_renderer.hpp"
#include "engine/renderer/shader.hpp"

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
    Shader shader;
    Camera *camera;

    float m_window_width, m_window_height;
    GLFWwindow *window;


    unsigned int VAO, VAO_next;
    unsigned int VBO, VBO_next;
    unsigned int EBO, EBO_next;

    std::vector<Vertex> vertex_buffer;
    std::vector<int> indicies_buffer;

    Text_Renderer text_renderer;

    std::vector<Vertex> verticies = {
        {100.0f, 100.0f, Color(255, 255, 0, 1.0f)},
        {150.0f, 100.0f, Color(255, 255, 0, 1.0f)},
        {100.0f, 150.0f, Color(255, 255, 0, 1.0f)}};
        
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

    GLFWwindow *create_window();
    GLFWwindow *get_window();

    void set_camera(Camera *camera);
    void update_camera_uniforms();

    void enable_blending();
    void enable_ortho_projection();

    void update_vertex_buffer(std::vector<Vertex> verticies);
    void index_buffer(std::vector<int> indicies);

    bool should_close();

    void cleanup();

    // debug
    void print_render_info();
    void checkGLError(const char *operation);

    // will render everthing
    bool render_everything();

    // will render the background
    void render_background();

    // will render the world (chunks in the future) and the particles
    void render_world();

    // will render the player and other game entities
    void render_entities();

    // the efects like explosions would be here
    // maybe will delete this and move it to render entities?
    void render_effects();

    // render user interface like buttons, invetnory health etc.
    void render_gui();
};