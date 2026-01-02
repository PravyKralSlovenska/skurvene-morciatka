#include "engine/renderer/buffers/buffer.hpp"

void Buffer::bind() {}

void Buffer::unbind() {}

bool Buffer::is_valid() const
{
    return id != 0 && glIsBuffer(id);
}

bool Buffer::is_binded(const int binding) const
{
    int current_buffer_id = 0;
    glGetIntegerv(binding, &current_buffer_id);
    return current_buffer_id == id;
}