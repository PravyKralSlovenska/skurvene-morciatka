// OpenGL a GLAD
#include "glad/gl.h"
#include <GLFW/glfw3.h>

// Standartne cpp kniznice
#include <iostream>
#include <fstream>
#include <sstream>

// Moje header files
#include "engine/world.hpp"
#include "src/world.cpp"
#include "engine/particles.hpp"

// konstanty
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;
const int PARTICLE_SIZE = 10;

static std::string read_file(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    // std::cout << ss.str() << std::endl;
    return ss.str();
}

static unsigned int compile_shader(unsigned int type, const std::string &source)
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

static unsigned int create_shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
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
        // x,    y,    r,    g,    b
        -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,

        -0.2f, -0.2f, 1.0f, 0.0f, 0.0f,
         0.2f, -0.2f, 0.0f, 1.0f, 0.0f,
         0.2f,  0.2f, 1.0f, 1.0f, 0.0f,
        -0.2f,  0.2f, 1.0f, 0.0f, 1.0f,
    };

    // Element Buffer Object - EBO
    unsigned int indices[12] = {
        0, 1, 3, // pravy trojuholnik
        1, 2, 3, // lavy trojuholnik

        4, 5, 6,
        4, 6, 7
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
     * 0 - attribute location (index)
     * 2 - velkost (x,y)
     * GL_FLOAT - data typ dat
     * GL_FALSE - normalizacia dat?
     * 5 - stride (kolko bajtov zaberie jeden vertex)
     * (void *)0 - offset (odkial zacina data pre tento atribut)
     */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);

    /*
     * glEnableVertexAttribArray
     * 0 - index atributu, ktory chceme zapnut
     */
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // cesty treba upravit podla togo kde skonci skompilovany kod ("morciatko")
    unsigned int program_shader = create_shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");

    glUseProgram(program_shader);

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
