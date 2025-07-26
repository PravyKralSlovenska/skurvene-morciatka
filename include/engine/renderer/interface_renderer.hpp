#pragma once

#include <iostream>

#include "engine/renderer/renderer.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/renderer/text_renderer.hpp"

class IRenderer
{
private:
    unsigned VBO, VAO;

    Renderer *renderer;
    Text_Renderer *text_renderer;

public:
    IRenderer();
    virtual ~IRenderer() = default;
    virtual void init() = 0;
    virtual void render() = 0;
    virtual void cleanup() = 0;

    void manage_projection_matrix();
};
