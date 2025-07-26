#include <iostream>
#include <vector>

#include <glad/gl.h>

#include "engine/renderer/buffer.hpp"

Buffer::Buffer()
    : ID(0) {}

Buffer::~Buffer()
{
}

VERTEX_ARRAY_OBJECT::VERTEX_ARRAY_OBJECT()
{
    glGenVertexArrays(1, &ID);
}

VERTEX_ARRAY_OBJECT::~VERTEX_ARRAY_OBJECT()
{
    glDeleteVertexArrays(1, &ID);
}

void VERTEX_ARRAY_OBJECT::bind()
{
    glBindVertexArray(ID);
}

void VERTEX_ARRAY_OBJECT::unbind()
{
    glBindVertexArray(0);
}

void VERTEX_ARRAY_OBJECT::setup_vertex_attribute_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    /*
     * glVertexAttribPointer
     * index - attribute location (index) - to znamena ze
     * size - velkost (x,y)
     * GL_FLOAT - data typ dat
     * GL_FALSE - normalizacia dat < -1; 1>
     * 4 * sizeof(float) - stride (kolko bajtov zaberie jeden vertex) (kolko bajtov
     *  . je od jedneho vertexu k druhemu) kazdy element v arraye ma 4 bajty (float),
     *  . takze 5 * sizeof(float) = 20 bajtov
     * (void *)0 - offset (odkial zacina data pre tento atribut)
     */
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    glEnableVertexAttribArray(index);
}

void VERTEX_ARRAY_OBJECT::disable_vertex_attribute_pointer(GLuint index)
{
    glDisableVertexAttribArray(index);
}

VERTEX_BUFFER_OBJECT::VERTEX_BUFFER_OBJECT()
{
    glGenBuffers(1, &ID);
    if (ID == 0) {
        std::cerr << "Failed to generate vertex buffer!\n";
    }
}

VERTEX_BUFFER_OBJECT::~VERTEX_BUFFER_OBJECT()
{
    if (ID != 0) {
        glDeleteBuffers(1, &ID);
        ID = 0;
    }
}

void VERTEX_BUFFER_OBJECT::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VERTEX_BUFFER_OBJECT::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ELEMENT_ARRAY_BUFFER::ELEMENT_ARRAY_BUFFER()
{
    glGenBuffers(1, &ID);
    if (ID == 0) {
        std::cerr << "Failed to generate vertex buffer!\n";
    }
}

ELEMENT_ARRAY_BUFFER::~ELEMENT_ARRAY_BUFFER()
{
    if (ID != 0) {
        glDeleteBuffers(1, &ID);
        ID = 0;
    }
}

void ELEMENT_ARRAY_BUFFER::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
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