#pragma once

#include "engine/renderer/buffers/buffer.hpp"

/*
 * VERTEX ARRAY OBJECT (VAO):
 * - stores bindings and enables for vertex attributes as well as the binding for the index list
 * - does not store actual data content, but stores references to arrays/buffers
 * - arrays/buffers must be created first before binding to VAO
 * - BIND FIRST
 * -  SECOND
 */
class VERTEX_ARRAY_OBJECT : public Buffer
{
public:
    VERTEX_ARRAY_OBJECT();
    ~VERTEX_ARRAY_OBJECT();

    void bind() override;
    void unbind() override;

    /*
     * glVertexAttribPointer
     * @param index attribute location (index) - to znamena ze
     * @param size velkost (x,y)
     * @param type GL_FLOAT data typ dat
     * @param normalized GL_FALSE normalizacia dat < -1; 1>
     * @param stride 4 * sizeof(float) - (kolko bajtov zaberie jeden vertex) (kolko bajtov
     *  . je od jedneho vertexu k druhemu) kazdy element v arraye ma 4 bajty (float),
     *  . takze 5 * sizeof(float) = 20 bajtov
     * @param offset - (void *)0 - (odkial zacina data pre tento atribut)
     */
    void setup_vertex_attribute_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
    void disable_vertex_attribute_pointer(GLuint index);
};