#include <iostream>
#include <sstream>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer.hpp"
#include "others/utils.hpp"

Vertex::Vertex() {}
Vertex::Vertex(float x, float y, Color color)
    : x(x), y(y), color(color) {}

Renderer::Renderer(float window_width, float window_height)
    : m_window_width(window_width), m_window_height(window_height) {}

void Renderer::init()
{
    init_glfw();
    window = create_window();
    init_glad();

    program_shader = create_shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");

    VAO = create_vertex_array_buffer();
    VBO = create_vertex_buffer_object();
    EBO = create_element_buffer_object();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::init_glfw()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void Renderer::init_glad()
{
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return;
    }
}

GLFWwindow *Renderer::create_window()
{
    GLFWwindow *window = glfwCreateWindow(m_window_width, m_window_height, "Morciatko", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create glfw window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    return window;
}

GLFWwindow *Renderer::get_window()
{
    return window;
}

void Renderer::set_camera(Camera *camera)
{
    this->camera = camera;

    render_info.push_back("camera was set");
}

void Renderer::update_camera_uniforms()
{
    glm::mat4 view_projection = camera->get_view_projection_matrix();
    unsigned int viewProjLoc = glGetUniformLocation(program_shader, "view_projection");
    glUseProgram(program_shader);
    glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, glm::value_ptr(view_projection));
}

unsigned int Renderer::compile_shader(unsigned int type, const std::string &source)
{
    unsigned int shader_id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader_id, 1, &src, nullptr);
    glCompileShader(shader_id);

    int result;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        char *msg = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(shader_id, length, &length, msg);
        std::cerr << "Failed to compile shader: "
                  << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
                  << "\n"
                  << msg << std::endl;
        glDeleteShader(shader_id);
        return -1;
    }
    return shader_id;
}

unsigned int Renderer::create_shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compile_shader(GL_VERTEX_SHADER, read_file(vertex_shader_path));
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, read_file(fragment_shader_path));

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Renderer::enable_blending()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render_info.push_back("blending enabled");
}

void Renderer::enable_ortho_projection()
{
    if (camera)
    {
        update_camera_uniforms();
    }
    else
    {
        glm::mat4 projection = glm::ortho(0.0f, m_window_width, m_window_height, 0.0f);
        unsigned int projLoc = glGetUniformLocation(program_shader, "projection");
        glUseProgram(program_shader);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }

    render_info.push_back("ortho projection enabled");
}

unsigned int Renderer::create_vertex_buffer_object()
{
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    render_info.push_back("vertex buffer object created");

    return VBO;
}

unsigned int Renderer::create_vertex_array_buffer()
{
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    render_info.push_back("vertex array object created");

    return VAO;
}

unsigned int Renderer::create_element_buffer_object()
{
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    render_info.push_back("element buffer object created");

    return EBO;
}

void Renderer::update_vertex_buffer(std::vector<Vertex> verticies)
{
    /*
     * glVertexAttribPointer
     * 0 - attribute location (index) - to znamena ze
     * 2 - velkost (x,y)
     * GL_FLOAT - data typ dat
     * GL_FALSE - normalizacia dat?
     * 5 * sizeof(float) - stride (kolko bajtov zaberie jeden vertex) (kolko bajtov
     *  . je od jedneho vertexu k druhemu) kazdy element v arraye ma 4 bajty (float),
     *  . takze 5 * sizeof(float) = 20 bajtov
     * (void *)0 - offset (odkial zacina data pre tento atribut)
     */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(verticies), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::index_buffer(std::vector<int> indicies)
{
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool Renderer::render_everything()
{
    // toto bude surovy backround background (asi)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (camera)
    {
        update_camera_uniforms();
    }

    render_background();

    render_world();

    render_entities();

    render_effects();
    
    render_gui();

    // update_vertex_buffer(vertex_buffer);
    // index_buffer(indicies_buffer);

    // glBindVertexArray(VAO);
    // glDrawElements(GL_TRIANGLES, indicies_buffer.size(), GL_UNSIGNED_INT, 0);

    // glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();

    return 1;
}

void Renderer::render_background()
{
}

void Renderer::render_world()
{
}

void Renderer::render_entities()
{
}

void Renderer::render_effects()
{
}

void Renderer::render_gui()
{
}

bool Renderer::should_close()
{
    return glfwWindowShouldClose(window);
}

void Renderer::cleanup()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(program_shader);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::print_render_info()
{
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

    for (std::string info : render_info)
    {
        std::cout << info << '\n';
    }
}

void Renderer::checkGLError(const char *operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error after " << operation << ": " << error << std::endl;
    }
}
