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

#include "others/utils.hpp"

Vertex::Vertex() {}
Vertex::Vertex(float x, float y, Color color)
    : x(x), y(y), color(color) {}

Renderer::Renderer(float window_width, float window_height)
    : m_window_width(window_width), m_window_height(window_height) {}

void Renderer::init()
{
    init_glfw();
    create_window();
    init_glad();

    // init vsetky render
    text_renderer = std::make_unique<Text_Renderer>();
    text_renderer->init();
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

void Renderer::create_window()
{
    window = glfwCreateWindow(m_window_width, m_window_height, "Morciatko", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create glfw window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
}

GLFWwindow *Renderer::get_window()
{
    return window;
}

bool Renderer::should_close()
{
    return glfwWindowShouldClose(window);
}

void Renderer::update_camera_uniforms()
{
    // glm::mat4 view_projection = camera.get_view_projection_matrix();
    // unsigned int viewProjLoc = glGetUniformLocation(shader.ID, "view_projection");
    // shader.use();
    // glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, glm::value_ptr(view_projection));
    // shader.set_mat4("view_projection", view_projection);
}

void Renderer::enable_blending()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render_info.push_back("blending enabled");
}

void Renderer::enable_ortho_projection()
{
    // if (camera)
    // {
    //     update_camera_uniforms();
    // }
    // else
    // {
    glm::mat4 projection = glm::ortho(0.0f, m_window_width, m_window_height, 0.0f);
    // unsigned int projLoc = glGetUniformLocation(shader.ID, "projection");
    // shader.use();
    // glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    // shader.set_mat4("projection", projection);
    // }

    render_info.push_back("ortho projection enabled");
}

void Renderer::enable_pixel_perfect_rendering()
{
    // Disable antialiasing
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    
    // Use nearest neighbor filtering for textures (if you have any)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Ensure pixel-aligned coordinates
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    render_info.push_back("pixel-perfect rendering enabled");
}

bool Renderer::render_everything()
{
    // toto bude surovy backround background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // update camera
    update_camera_uniforms();

    // render renders

    text_renderer->render_text();
    // text_renderer->render_triangle();

    glfwSwapBuffers(window);
    glfwPollEvents();

    return 1;
}

void Renderer::cleanup()
{
    // text_renderer.~Text_Renderer();

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
