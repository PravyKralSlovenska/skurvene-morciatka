// OpenGL a GLAD
#include "glad/gl.h"
#include <GLFW/glfw3.h>

// Standartne cpp kniznice
#include <iostream>
#include <fstream>
#include <sstream>

// Moje header files
#include "engine/world.hpp"
#include "engine/particles.hpp"
#include "others/utils.hpp"

// konstanty
const int WINDOW_WIDTH = 1800;
const int WINDOW_HEIGHT = 1000;
const int PARTICLE_SIZE = 50;

World world(WINDOW_WIDTH, WINDOW_HEIGHT, PARTICLE_SIZE);

int main(int argc, char **argv)
{
    // init GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // vytvorenie okna
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Morciatko", nullptr, nullptr);
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

    // Vertex Array Object - VAO
    float vertices[] = {
        // X,    Y,    R,    G,    B,    A
        // 0,    1,    2,    3,    4,    5
        -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,

        -0.2f, -0.2f, 1.0f, 0.0f, 0.0f, 0.2f,
         0.2f, -0.2f, 0.0f, 1.0f, 0.0f, 0.2f,
         0.2f,  0.2f, 0.0f, 0.0f, 1.0f, 0.2f,
        -0.2f,  0.2f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    // Element Buffer Object - EBO
    unsigned int indices[] = {
        0, 1, 3, // pravy trojuholnik
        1, 2, 3, // lavy trojuholnik

        4, 5, 7, // pravy trojuholnik
        5, 6, 7, // lavy trojuholnik
    };

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /*
     * EBO (Element Buffer Object) - sluzi na ukladanie indexov pre vertexy
     */
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /*
     * glVertexAttribPointer
     * 0 - attribute location (index) - to znamena ze
     * 2 - velkost (x,y)
     * GL_FLOAT - data typ dat
     * GL_FALSE - normalizacia dat?
     * 5 * sizeof(float) - stride (kolko bajtov zaberie jeden vertex) (kolko bajtov 
     *  . je od jedneho vertixu k druhemu) kazdy element v arraye ma 4 bajty (float), 
     *  . takze 5 * sizeof(float) = 20 bajtov
     * (void *)0 - offset (odkial zacina data pre tento atribut)
     */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    /*
     * glEnableVertexAttribArray
     * 0 - index atributu, ktory chceme zapnut
     */

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // cesty treba upravit podla togo kde skonci skompilovany kod ("morciatko")
    unsigned int program_shader = create_shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");

    glUseProgram(program_shader);
    
    // blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // double xpos, ypos;
        // glfwGetCursorPos(window, &xpos, &ypos);
        // std::cout << "Mouse Position: " << xpos << ", " << ypos << std::endl;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            
            std::cout << "Mouse Click: " << (int)xpos / PARTICLE_SIZE << ", " << (int)ypos / PARTICLE_SIZE << std::endl;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        // glDrawArrays(GL_TRIANGLES, 0, 4);
        glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);

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
