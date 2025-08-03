#include <iostream>
#include <memory>

#include "engine/entity.hpp"
#include "engine/renderer/buffer.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/renderer/entities_renderer.hpp"

Entities_Renderer::Entities_Renderer() {}
Entities_Renderer::~Entities_Renderer() {}

void Entities_Renderer::init()
{
    VAO = std::make_unique<VERTEX_ARRAY_OBJECT>();
    VBO = std::make_unique<VERTEX_BUFFER_OBJECT>();

    shader = std::make_unique<Shader>("", "");
}

void Entities_Renderer::add_entity(Entity *entity)
{

}

void Entities_Renderer::render_entities(const std::vector<Entity> &entities)
{
    // shader->use();


    // std::cout << entities.size() << '\n';
}
