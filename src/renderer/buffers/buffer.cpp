#include "engine/renderer/buffers/buffer.hpp"

void Buffer::bind() {}

void Buffer::unbind() {}

bool Buffer::is_valid() const
{
    return id != 0 && glIsBuffer(id);
}
