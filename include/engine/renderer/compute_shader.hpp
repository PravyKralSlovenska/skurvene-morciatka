#pragma once

#include <iostream>
#include <glm/glm.hpp>

/*
 * Compute Shader
 *
 *
 */
class Compute_Shader
{
private:
    const std::string compute_path;

    // const int work_group;

private:
    void init();
    // void compile_shader();
    void check_compile_errors(const unsigned int shader, const std::string type);

public:
    unsigned int ID;
    unsigned int compute;

public:
    Compute_Shader(const std::string &compute_path);
    ~Compute_Shader();

    void use();

    void dispatch(const int x, const int y, const int z = 1);
    // void get_dispatch() const;

    void set_memory_barrier(const unsigned int type);

    void set_work_group(const int size) const;
    void set_bool(const std::string &name, bool value) const;
    void set_int(const std::string &name, int value) const;
    void set_float(const std::string &name, float value) const;
    void set_vec2(const std::string &name, const glm::vec2 &value) const;
    void set_vec2(const std::string &name, float x, float y) const;
    void set_vec3(const std::string &name, const glm::vec3 &value) const;
    void set_vec3(const std::string &name, float x, float y, float z) const;
    void set_vec4(const std::string &name, const glm::vec4 &value) const;
    void set_vec4(const std::string &name, float x, float y, float z, float w) const;
    void set_mat2(const std::string &name, const glm::mat2 &mat) const;
    void set_mat3(const std::string &name, const glm::mat3 &mat) const;
    void set_mat4(const std::string &name, const glm::mat4 &mat) const;
};
