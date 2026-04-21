#pragma once

// File purpose: Defines vertex buffer object (VBO) upload helpers.
#include <vector>

#include "engine/renderer/buffers/buffer.hpp"

/*
 * VERTEX BUFFER OBJECT (VBO):
 * - contains raw data as a list of bytes
 * - tells GPU how to pull data out of buffer (offsets, data types, interleaving, etc.)
 * - order of binding VBOs and VAOs is relevant, as binding VBOs changes the bound VAO
 * - BIND SECOND
 * - FILL FIRST
 */
// Wraps vertex buffer uploads and binding.
class VERTEX_BUFFER_OBJECT : public Buffer
{
public:
    // Constructs VERTEX_BUFFER_OBJECT.
    VERTEX_BUFFER_OBJECT();
    // Destroys VERTEX_BUFFER_OBJECT and releases owned resources.
    ~VERTEX_BUFFER_OBJECT();

    // Binds.
    void bind() override;
    // Unbinds.
    void unbind() override;

    template <typename T>
    void fill_with_data_vector(const std::vector<T> &vertices, GL_DRAW draw)
    {
        // Binds.
        bind();
        // Gl Buffer Data.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), draw);
    }

    void fill_with_data_raw(unsigned int size, const GLvoid *data, GLenum draw)
    {
        // Binds.
        bind();
        // Gl Buffer Data.
        glBufferData(GL_ARRAY_BUFFER, size, data, draw);
    }
};
