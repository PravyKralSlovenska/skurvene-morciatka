#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "others/utils.hpp"

Color::Color() {}
Color::Color(int r, int g, int b, float a)
    : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a) {}

/*
 *
 */
std::string read_file(const std::string &filepath)
{
    // std::string font_name = FileSystem::getPath();
    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

/*
 * z utils.hpp
 * Skontroluje, ci je hodnota v rozsahu
 * Pouzivam v particles.hpp
 */
bool in_world_range(int x, int y, int world_rows, int world_cols)
{
    return (x >= 0 && x < world_cols) && (y >= 0 && y < world_rows);
}

unsigned int create_vertex_buffer_object()
{
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    std::cout << "VBO vytvoreny " << VBO << "\n";

    return VBO;
}

unsigned int create_vertex_array_buffer()
{
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    std::cout << "VAO vytvoreny " << VAO << "\n";


    return VAO;
}

unsigned create_element_buffer_object()
{
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    std::cout << "EBO vytvoreny " << EBO << "\n";


    return EBO;
}

unsigned int compile_shader(unsigned int type, const std::string &source)
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

unsigned int create_shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
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