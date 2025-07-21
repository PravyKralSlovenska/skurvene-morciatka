#pragma once
#include <iostream>
#include <vector>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "others/utils.hpp"
#include "engine/camera.hpp"

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
    // cisto len na infosky
    std::vector<std::string> render_info;

    Camera *camera;

    float m_window_width, m_window_height;
    GLFWwindow *window;

    unsigned int program_shader;

    unsigned int VAO, VAO_next;
    unsigned int VBO, VBO_next;
    unsigned int EBO, EBO_next;

    std::vector<Vertex> vertex_buffer;
    std::vector<int> indicies_buffer;

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

    unsigned int create_vertex_array_buffer();
    unsigned int create_vertex_buffer_object();
    unsigned int create_element_buffer_object();

    void update_vertex_buffer(std::vector<Vertex> verticies);
    void index_buffer(std::vector<int> indicies);

    unsigned int create_shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);
    unsigned int compile_shader(unsigned int type, const std::string &source);

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