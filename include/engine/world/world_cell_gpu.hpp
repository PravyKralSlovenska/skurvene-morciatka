#pragma once

#include <cstdint>

#include <glm/glm.hpp>

struct GPUWorldCell
{
    glm::ivec2 coords{0, 0};
    glm::ivec2 padding0{0, 0};
    glm::vec4 base_color{0.0f};
    glm::vec4 color{0.0f};
    glm::uvec4 meta{0u};
};
