#include <iostream>

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/shader.hpp"
#include "others/utils.hpp"

Shader::Shader() {}

Shader::Shader(const std::string &vertex_path, const std::string &fragment_path)
    : vertex_path(vertex_path), fragment_path(fragment_path)
{
    create_shader();
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::create_shader()
{
    ID = glCreateProgram();
    if (ID == 0)
    {
        std::cerr << "ERROR::SHADER: Failed to create shader program\n";
        return;
    }

    std::string vertex_source = read_file(vertex_path);
    if (vertex_source.empty())
    {
        std::cerr << "ERROR::SHADER: Failed to read vertex shader file: " << vertex_path << std::endl;
        return;
    }

    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertex_source);
    if (vs == -1)
    {
        std::cerr << "ERROR::SHADER: Failed to compile vertex shader\n";
        return;
    }

    std::string fragment_source = read_file(fragment_path);
    if (fragment_source.empty())
    {
        std::cerr << "ERROR::SHADER: Failed to read fragment shader file: " << fragment_path << std::endl;
        glDeleteShader(vs);
        return;
    }

    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    if (fs == -1)
    {
        std::cerr << "ERROR::SHADER: Failed to compile fragment shader\n";
        glDeleteShader(vs);
        return;
    }

    glAttachShader(ID, vs);
    glAttachShader(ID, fs);

    glLinkProgram(ID);
    glValidateProgram(ID);

    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    std::cout << "shader s ideckom " << ID << " je vytvoreny\n";
}

unsigned int Shader::compile_shader(unsigned int type, const std::string &source)
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

void Shader::set_bool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::set_int(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::set_float(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::set_vec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::set_vec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::set_vec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::set_mat2(const std::string &name, const glm::mat2 &value) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set_mat3(const std::string &name, const glm::mat3 &value) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set_mat4(const std::string &name, const glm::mat4 &value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
