#include "engine/renderer/buffers/shader_storage_buffer_object.hpp"

Shader_Storage_Buffer_Object::Shader_Storage_Buffer_Object()
{
    glGenBuffers(1, &id);
}

Shader_Storage_Buffer_Object::~Shader_Storage_Buffer_Object()
{
    if (id != 0)
    {
        glDeleteBuffers(1, &id);
        id = 0;
    }
}

void Shader_Storage_Buffer_Object::bind()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
}

void Shader_Storage_Buffer_Object::bind_base(const unsigned int binding_point)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, id);
}

void Shader_Storage_Buffer_Object::unbind()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Shader_Storage_Buffer_Object::allocate(size_t size_bytes, GL_DRAW draw)
{
    bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size_bytes, nullptr, draw);
}
