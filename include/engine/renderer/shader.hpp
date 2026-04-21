#pragma once

// File purpose: Defines GLSL shader program loading and uniform helpers.
#include <iostream>
#include <glm/glm.hpp>

// Wraps GLSL program compilation and uniform updates.
class Shader
{
private:
    const std::string vertex_path, fragment_path;

public:
    unsigned int ID;

public:
    // Constructs Shader.
    Shader();
    // Constructs Shader.
    Shader(const std::string &vertex_path, const std::string &fragment_path);
    // Destroys Shader and releases owned resources.
    ~Shader();

    // Uses.
    void use();
    // Creates shader.
    void create_shader();
    // Compiles shader.
    unsigned int compile_shader(unsigned int type, const std::string &source);

    // Returns true if binded.
    bool is_binded();

    // setters
    void set_int(const std::string &name, int value) const;
    // Sets bool.
    void set_bool(const std::string &name, bool value) const;
    // Sets float.
    void set_float(const std::string &name, float value) const;

    // Sets vec2.
    void set_vec2(const std::string &name, const glm::vec2 &value) const;
    // void set_vec2(const std::string &name, const float x, const float y) const;

    void set_vec3(const std::string &name, const glm::vec3 &value) const;
    // void set_vec3(const std::string &name, const float x, const float y, const float z) const;

    void set_vec4(const std::string &name, const glm::vec4 &value) const;
    // void set_vec4(const std::string &name, const float x, const float y, const float z, const float w) const;

    void set_mat2(const std::string &name, const glm::mat2 &value) const;
    // void set_mat2(const std::string &name, const float x, const float y) const;

    void set_mat3(const std::string &name, const glm::mat3 &value) const;
    // void set_mat3(const std::string &name, const float x, const float y, const float z) const;

    void set_mat4(const std::string &name, const glm::mat4 &value) const;
    // void set_mat4(const std::string &name, const float x, const float y, const float z, const float w) const;

    // void set_projection(const std::string &name, const glm::mat4 &projection); ?
};
