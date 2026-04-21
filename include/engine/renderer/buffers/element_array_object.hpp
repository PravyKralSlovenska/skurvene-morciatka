#pragma once

// File purpose: Defines index buffer (EBO) upload and binding helpers.
#include <vector>

#include "engine/renderer/buffers/buffer.hpp"

/*
 * ELEMENT ARRAY BUFFER (EBO):
 * - buffer object used for storing index data for DrawElements
 * - binds to different bind point than VBO so GPU pulls index data from it
 * - essentially same as VBO but used for indices instead of vertex attributes
 * - BIND THIRD
 */
// Wraps index-buffer uploads and binding.
class ELEMENT_ARRAY_BUFFER : public Buffer
{
public:
    // Constructs ELEMENT_ARRAY_BUFFER.
    ELEMENT_ARRAY_BUFFER();
    // Destroys ELEMENT_ARRAY_BUFFER and releases owned resources.
    ~ELEMENT_ARRAY_BUFFER();

    // Binds.
    void bind() override;
    // Unbinds.
    void unbind() override;

    // Fills with data.
    void fill_with_data(const std::vector<unsigned int> &indices, GL_DRAW draw = GL_STATIC);
};
