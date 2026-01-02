#include "engine/renderer/compute_shader.hpp"

#include <glad/gl.h>

#include "others/utils.hpp"

Compute_Shader::Compute_Shader(const std::string &compute_path)
    : compute_path(compute_path)
{
    init();
}

Compute_Shader::~Compute_Shader()
{
    glDeleteProgram(ID);
}

void Compute_Shader::init()
{
    std::string compute_source = read_file(compute_path);
    if (compute_source.empty())
    {
        std::cerr << "COMPUTE SHADER sa nepodarilo nacitat id: " << ID << '\n';
        return;
    }

    const char *compute_code = compute_source.c_str();

    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &compute_code, nullptr);
    glCompileShader(compute);
    check_compile_errors(compute, "COMPUTE");

    ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    check_compile_errors(ID, "PROGRAM");

    glDeleteShader(compute);

    std::cout << "COMPUTE SHADER JE VYTVORENY S ID: " << ID << '\n';
}

bool Compute_Shader::is_binded()
{
    int current_shader_id = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_shader_id);
    return current_shader_id == ID; // vrati true ak sucasny program je pripojeny
}

void Compute_Shader::use()
{
    if (is_binded())
    {
        return;
    }

    glUseProgram(ID);
}

void Compute_Shader::dispatch(const int x, const int y, const int z)
{
    glDispatchCompute(x, y, z);
}

void Compute_Shader::set_memory_barrier(const unsigned int type)
{
    glMemoryBarrier(type);
}

void Compute_Shader::set_work_group(const int size) const
{
    // mozno niekedy
}

void Compute_Shader::set_bool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Compute_Shader::set_int(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Compute_Shader::set_float(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Compute_Shader::set_vec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Compute_Shader::set_vec2(const std::string &name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Compute_Shader::set_ivec2(const std::string &name, const glm::ivec2 &value) const
{
    glUniform2iv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Compute_Shader::set_ivec2(const std::string &name, const int x, const int y) const
{
    glUniform2i(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Compute_Shader::set_vec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Compute_Shader::set_vec3(const std::string &name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Compute_Shader::set_vec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Compute_Shader::set_vec4(const std::string &name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Compute_Shader::set_mat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Compute_Shader::set_mat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Compute_Shader::set_mat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Compute_Shader::check_compile_errors(const unsigned int shader, const std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
