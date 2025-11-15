#pragma once

#include <vector>

#include "engine/renderer/buffers/buffer.hpp"

/*
 * ELEMENT ARRAY BUFFER (EBO):
 * - buffer object used for storing index data for DrawElements
 * - binds to different bind point than VBO so GPU pulls index data from it
 * - essentially same as VBO but used for indices instead of vertex attributes
 * - BIND THIRD
 */
class ELEMENT_ARRAY_BUFFER : public Buffer
{
public:
    ELEMENT_ARRAY_BUFFER();
    ~ELEMENT_ARRAY_BUFFER();

    void bind() override;
    void unbind() override;

    void fill_with_data(const std::vector<unsigned int> &indices, GL_DRAW draw = GL_STATIC);
};
