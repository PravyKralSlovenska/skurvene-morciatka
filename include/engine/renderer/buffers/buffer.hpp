#pragma once

#include <glad/gl.h>

/*
 * BUFFER:
 * - base class for other classes
 */
class Buffer
{
public:
    unsigned int id;

public:
    virtual void bind() = 0;
    virtual void unbind() = 0;

    bool is_valid() const;
};

// this for VERTEX_BUFFER_OBJECT on how should be the thing drawed
// tells GPU how to behave too
// je to vobec treba??? :-P
enum GL_DRAW
{
    GL_STATIC = GL_STATIC_DRAW,  // THE BUFFER DATA IS ONLY SET ONCE
    GL_STREAM = GL_STREAM_DRAW,  // THE BUFFER DATA IS SET ONCE AND FEW TIMES LATER
    GL_DYNAMIC = GL_DYNAMIC_DRAW // THE BUFFER DATA IS CONSTATNLY CHANING
};

// dalsie buffery
// UBO
// SSBO
// SSAO
