#pragma once

// File purpose: Defines compute shader compilation and dispatch utilities.
#include <iostream>
#include <glm/glm.hpp>

/*
 * Compute Shader
 *
 *
 */
// Wraps compute shader compilation and dispatch.
class Compute_Shader
{
private:
    const std::string compute_path;

    // const int work_group;

private:
    // Initializes state.
    void init();
    // void compile_shader();
    void check_compile_errors(const unsigned int shader, const std::string type);

public:
    unsigned int ID;
    unsigned int compute;

public:
    // Constructs Compute_Shader.
    Compute_Shader(const std::string &compute_path);
    // Destroys Compute_Shader and releases owned resources.
    ~Compute_Shader();

    // Uses.
    void use();

    // Returns true if binded.
    bool is_binded();

    // Dispatches.
    void dispatch(const int x, const int y, const int z = 1);
    // void get_dispatch() const;

    void set_memory_barrier(const unsigned int type);
    // Sets work group.
    void set_work_group(const int size) const;

    // Sets bool.
    void set_bool(const std::string &name, bool value) const;
    // Sets int.
    void set_int(const std::string &name, int value) const;
    // Sets float.
    void set_float(const std::string &name, float value) const;
    // Sets vec2.
    void set_vec2(const std::string &name, const glm::vec2 &value) const;
    // Sets ivec2.
    void set_ivec2(const std::string &name, const glm::ivec2 &value) const;
    // Sets ivec2.
    void set_ivec2(const std::string &name, const int x, const int y) const;
    // Sets vec2.
    void set_vec2(const std::string &name, float x, float y) const;
    // Sets vec3.
    void set_vec3(const std::string &name, const glm::vec3 &value) const;
    // Sets vec3.
    void set_vec3(const std::string &name, float x, float y, float z) const;
    // Sets vec4.
    void set_vec4(const std::string &name, const glm::vec4 &value) const;
    // Sets vec4.
    void set_vec4(const std::string &name, float x, float y, float z, float w) const;
    // Sets mat2.
    void set_mat2(const std::string &name, const glm::mat2 &mat) const;
    // Sets mat3.
    void set_mat3(const std::string &name, const glm::mat3 &mat) const;
    // Sets mat4.
    void set_mat4(const std::string &name, const glm::mat4 &mat) const;
};
