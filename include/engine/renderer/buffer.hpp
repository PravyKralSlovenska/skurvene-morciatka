#pragma once

#include <iostream>
#include <vector>

#include <glad/gl.h>

/*
 * BUFFER:
 * - base class for other classes I think im fucked
 */
class Buffer
{
protected:
    unsigned int ID;

public:
    Buffer();
    virtual ~Buffer() = 0;

    virtual void bind() = 0;
    virtual void unbind() = 0;
};

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

    void setup_vertex_attribute_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
    void disable_vertex_attribute_pointer(GLuint index);
};

// this for VERTEX_BUFFER_OBJECT on how should be the thing drawed
// tells GPU how to behave too
// je to vobec treba :-P
enum GL_DRAW
{
    GL_STATIC = GL_STATIC_DRAW,  // THE BUFFER DATA IS ONLY SET ONCE
    GL_STREAM = GL_STREAM_DRAW,  // THE BUFFER DATA IS SET ONCE AND FEW TIMES LATER
    GL_DYNAMIC = GL_DYNAMIC_DRAW // THE BUFFER DATA IS CONSTATNLY CHANING
};

/*
 * VERTEX BUFFER OBJECT (VBO):
 * - contains raw data as a list of bytes
 * - tells GPU how to pull data out of buffer (offsets, data types, interleaving, etc.)
 * - order of binding VBOs and VAOs is relevant, as binding VBOs changes the bound VAO
 * - BIND SECOND
 * - FILL FIRST
 */
class VERTEX_BUFFER_OBJECT : public Buffer
{
public:
    VERTEX_BUFFER_OBJECT();
    ~VERTEX_BUFFER_OBJECT();

    void bind() override;
    void unbind() override;

    template <typename T>
    void fill_with_data_vector(const std::vector<T> &vertices, GL_DRAW draw)
    {
        bind();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), draw);
    }
    
    void fill_with_data_raw(unsigned int size, const GLvoid *data, GLenum draw)
    {
        bind();
        glBufferData(GL_ARRAY_BUFFER, size, data, draw);
    }
};

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
