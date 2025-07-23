#pragma once

#include <iostream>

#include <glm/glm.hpp>

class Shader
{
private:
    const std::string vertex_path, fragment_path;

public:
    unsigned int ID;

public:
    Shader();
    Shader(const std::string &vertex_path, const std::string &fragment_path);
    ~Shader();

    void use();
    void create_shader();
    unsigned int compile_shader(unsigned int type, const std::string &source);

    // setters
    void set_int(const std::string &name, int value) const;
    void set_bool(const std::string &name, bool value) const;
    void set_float(const std::string &name, float value) const;

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
