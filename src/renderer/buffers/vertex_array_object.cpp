#include "engine/renderer/buffers/vertex_array_object.hpp"

VERTEX_ARRAY_OBJECT::VERTEX_ARRAY_OBJECT()
{
    glGenVertexArrays(1, &id);
}

VERTEX_ARRAY_OBJECT::~VERTEX_ARRAY_OBJECT()
{
    glDeleteVertexArrays(1, &id);
}

void VERTEX_ARRAY_OBJECT::bind()
{
    if (is_binded(GL_VERTEX_ARRAY_BINDING))
    {
        return;
    }

    glBindVertexArray(id);
}

void VERTEX_ARRAY_OBJECT::unbind()
{
    glBindVertexArray(0);
}

void VERTEX_ARRAY_OBJECT::setup_vertex_attribute_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    glEnableVertexAttribArray(index);
}

void VERTEX_ARRAY_OBJECT::disable_vertex_attribute_pointer(GLuint index)
{
    glDisableVertexAttribArray(index);
}