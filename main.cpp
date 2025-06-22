// OpenGL a GLAD
#include "glad/gl.h"
#include <GLFW/glfw3.h>

// Standartne cpp kniznice
#include <iostream>
#include <fstream>
#include <sstream>

// Moje header files
#include "engine/grid.hpp"
#include "engine/particle.hpp"

// konstanty
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;
const int PARTICLE_SIZE = 50;

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
    float vertices[8] = {
        -0.5f, -0.5f, // dolny lavy roh
        0.5f, -0.5f,  // dolny pravy roh
        0.5f, 0.5f,   // horni pravy roh
        -0.5f, 0.5f   // horni lavy roh
    };

    // Element Buffer Object - EBO
    unsigned int indices[6] = {
        0, 1, 3, // pravy trojuholnik
        1, 2, 3  // lavy trojuholnik
    };

    // idcka pre VBO, VAO a EBO
    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    unsigned int program_shader = create_shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error: " << err << std::endl;
    }

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program_shader);
        glBindVertexArray(VAO);

        // glDrawArrays(GL_TRIANGLES, 0, 4);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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