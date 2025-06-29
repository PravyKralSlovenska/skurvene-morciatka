#include "glad/gl.h"

#include "others/utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

/*
 *
 */
std::string read_file(const std::string &filepath)
{
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
 *
 */
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

/*
 * Vyvori shader program z shaderov
 */
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
