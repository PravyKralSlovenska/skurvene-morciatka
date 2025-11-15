#include <iostream>

#include "engine/renderer/buffers/vertex_buffer_object.hpp"

VERTEX_BUFFER_OBJECT::VERTEX_BUFFER_OBJECT()
{
    glGenBuffers(1, &id);
    if (id == 0)
    {
        std::cerr << "Failed to generate vertex buffer!\n";
    }
}

VERTEX_BUFFER_OBJECT::~VERTEX_BUFFER_OBJECT()
{
    if (id != 0)
    {
        glDeleteBuffers(1, &id);
        id = 0;
    }
}

void VERTEX_BUFFER_OBJECT::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VERTEX_BUFFER_OBJECT::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}