#pragma once

#include <vector>

#include "engine/renderer/buffers/buffer.hpp"

/*
 * Shader_Storage_Buffer_Object (SSBO)
 */
class Shader_Storage_Buffer_Object : public Buffer
{
public:
    Shader_Storage_Buffer_Object();
    ~Shader_Storage_Buffer_Object();

    void bind() override;
    void bind_base(const unsigned int binding_point);
    void unbind() override;

    void allocate(size_t size_bytes, GL_DRAW draw);

    template <typename T>
    void fill_with_data(const std::vector<T> &data, GL_DRAW draw)
    {
        bind();
        glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), draw);
    }

    /*
     * @param offset musi byt v bytoch
     */
    template <typename T>
    void fill_with_sub_data(const std::vector<T> &data, const int offset)
    {
        bind();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, data.size() * sizeof(T), data.data());
    }

    void get_the_data(); // mozno
};