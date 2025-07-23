#include <iostream>
#include <sstream>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/renderer.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/renderer/text_renderer.hpp"

#include "others/utils.hpp"

Vertex::Vertex() {}
Vertex::Vertex(float x, float y, Color color)
    : x(x), y(y), color(color) {}

Renderer::Renderer(float window_width, float window_height)
    : shader("../shaders/vertex.glsl", "../shaders/fragment.glsl"), m_window_width(window_width), m_window_height(window_height) {}

void Renderer::init()
{
    init_glfw();
    window = create_window();
    init_glad();

    VAO = create_vertex_array_buffer();
    VBO = create_vertex_buffer_object();
    EBO = create_element_buffer_object();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // init vsetky rendere
    text_renderer.init();
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
    // unsigned int viewProjLoc = glGetUniformLocation(shader.ID, "view_projection");
    shader.use();
    // glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, glm::value_ptr(view_projection));
    shader.set_mat4("view_projection", view_projection);
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
        // unsigned int projLoc = glGetUniformLocation(shader.ID, "projection");
        shader.use();
        // glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        shader.set_mat4("projection", projection);
    }

    render_info.push_back("ortho projection enabled");
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
    // toto bude surovy backround background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (camera)
    {
        update_camera_uniforms();
    }

    // // Render triangle using verticies vector
    // shader.use();
    
    // // Update vertex buffer with triangle data
    // glBindVertexArray(VAO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(Vertex), verticies.data(), GL_DYNAMIC_DRAW);
    
    // // Setup vertex attributes for position (location 0)
    // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // glEnableVertexAttribArray(0);
    
    // // Setup vertex attributes for color (location 1)
    // glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
    // glEnableVertexAttribArray(1);
    
    // // Draw the triangle
    // glDrawArrays(GL_TRIANGLES, 0, verticies.size());
    
    // glBindVertexArray(0);

    // render_background();

    // render_world();

    // render_entities();

    // render_effects();
    
    // render_gui();

    text_renderer.render_text();

    // update_vertex_buffer(vertex_buffer);
    // index_buffer(indicies_buffer);

    // glBindVertexArray(VAO);
    // glDrawElements(GL_TRIANGLES, indicies_buffer.size(), GL_UNSIGNED_INT, 0);

    // glBindVertexArray(0);
    // checkGLError("kokot");
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
