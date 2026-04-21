#pragma once

// File purpose: Defines shader storage buffer object (SSBO) helpers.
#include <vector>

#include "engine/renderer/buffers/buffer.hpp"

/*
 * Shader_Storage_Buffer_Object (SSBO)
 * 1. allocate
 * 2. bind_base
 */
// Wraps SSBO allocation, updates, and binding.
class Shader_Storage_Buffer_Object : public Buffer
{
public:
    // Constructs Shader_Storage_Buffer_Object.
    Shader_Storage_Buffer_Object();
    // Destroys Shader_Storage_Buffer_Object and releases owned resources.
    ~Shader_Storage_Buffer_Object();

    // Binds.
    void bind() override;
    // Binds base.
    void bind_base(const unsigned int binding_point);
    // Unbinds.
    void unbind() override;

    // index v compute shadery (bind_base)
    bool is_binded_based(const int index);

    // Allocate.
    void allocate(size_t size_bytes, GL_DRAW draw);

    template <typename T>
    void fill_with_data(const std::vector<T> &data, GL_DRAW draw)
    {
        // Binds.
        bind();
        // Gl Buffer Data.
        glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), draw);
    }

    /*
     * @param offset musi byt v bytoch
     */
    template <typename T>
    void fill_with_sub_data(const std::vector<T> &data, const int offset)
    {
        // Binds.
        bind();
        // Gl Buffer Sub Data.
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, data.size() * sizeof(T), data.data());
    }

    // Returns the data.
    void get_the_data(); // mozno
};
