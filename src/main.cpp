// OpenGL a GLAD
#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standartne cpp kniznice
#include <iostream>

// Moje header files
#include "engine/world.hpp"
#include "engine/particle.hpp"
#include "others/GLOBALS.hpp"
#include "others/utils.hpp"

void checkGLError(const char *operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error after " << operation << ": " << error << std::endl;
    }
}

int main(int argc, char **argv)
{
    World world(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE);

    // init GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // vytvorenie okna
    GLFWwindow *window = glfwCreateWindow(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, "Morciatko", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // init GLAD
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

    /*
     * Vertex Array Object (VAO) - sluzi na ukladanie konfiguracie vertexov
     */
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /*
     * Vertex Buffer Object (VBO) - sluzi na ukladanie dat vertexov
     */
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    /*
     * glBufferData
     * GL_ARRAY_BUFFER -
     * sizeof(vertices) - velkost dat, ktore sa budu ukladat do VBO
     * vertices - data, ktore sa budu ukladat do VBO
     * GL_STATIC_DRAW - typ pouzitia dat (static, dynamic, stream)
     */
    glBufferData(GL_ARRAY_BUFFER, world.vertex_buffer.size() * sizeof(Vertex), world.vertex_buffer.data(), GL_DYNAMIC_DRAW);

    /*
     * EBO (Element Buffer Object) - sluzi na ukladanie indexov pre vertexy
     */
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, world.indices.size() * sizeof(unsigned int), world.indices.data(), GL_DYNAMIC_DRAW);

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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(0);

    /*
     * glEnableVertexAttribArray
     * 0 - index atributu, ktory chceme zapnut
     */
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // cesty treba upravit podla togo kde skonci skompilovany kod ("morciatko")
    unsigned int program_shader = create_shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");

    // blending
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*
     * PROJECTION
     */
    glm::mat4 projection = glm::ortho(0.0f, Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, 0.0f);
    unsigned int projLoc = glGetUniformLocation(program_shader, "projection");
    glUseProgram(program_shader);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            world.clear_worlds();
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int xpos2 = (int)xpos / Globals::PARTICLE_SIZE;
            int ypos2 = (int)ypos / Globals::PARTICLE_SIZE;
            world.add_particle(Particle::create_sand(xpos2, ypos2, 0, 0), xpos2, ypos2);
        }

        world.update_world();

        if (!world.vertex_buffer.empty() && !world.indices.empty())
        {
            // std::cout << "Vertex buffer size: " << world.vertex_buffer.size() << std::endl;
            // std::cout << "Indices size: " << world.indices.size() << std::endl;
            // std::cout << "Drawing " << world.indices.size() / 3 << " triangles" << std::endl;

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, world.vertex_buffer.size() * sizeof(Vertex), world.vertex_buffer.data(), GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, world.indices.size() * sizeof(unsigned int), world.indices.data(), GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, world.indices.size(), GL_UNSIGNED_INT, 0);

            // checkGLError("glDrawElements");
        }

        world.clear_vertex_buffer();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(program_shader);
    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
