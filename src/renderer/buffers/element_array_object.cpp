#include <iostream>

#include "engine/renderer/buffers/element_array_object.hpp"

ELEMENT_ARRAY_BUFFER::ELEMENT_ARRAY_BUFFER()
{
    glGenBuffers(1, &id);
    if (id == 0)
    {
        std::cerr << "Failed to generate vertex buffer!\n";
    }
}

ELEMENT_ARRAY_BUFFER::~ELEMENT_ARRAY_BUFFER()
{
    if (id != 0)
    {
        glDeleteBuffers(1, &id);
        id = 0;
    }
}

void ELEMENT_ARRAY_BUFFER::bind()
{
    if (is_binded(GL_ELEMENT_ARRAY_BUFFER_BINDING))
    {
        return;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void ELEMENT_ARRAY_BUFFER::unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ELEMENT_ARRAY_BUFFER::fill_with_data(const std::vector<unsigned int> &indices, GL_DRAW draw)
{
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), draw);
}