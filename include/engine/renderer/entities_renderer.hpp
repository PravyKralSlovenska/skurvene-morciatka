#pragma once

#include <iostream>
#include <memory>

#include "engine/player/entity.hpp"
#include "engine/renderer/buffers/vertex_array_object.hpp"
#include "engine/renderer/buffers/vertex_buffer_object.hpp"
#include "engine/renderer/shader.hpp"

class Entities_Renderer
{
private:
    std::unique_ptr<VERTEX_ARRAY_OBJECT> VAO;
    std::unique_ptr<VERTEX_BUFFER_OBJECT> VBO;

    std::unique_ptr<Shader> shader;
    
public:
    Entities_Renderer();
    ~Entities_Renderer();
    
    void init();
    void add_entity(Entity *entity);
    void render_entities(const std::vector<Entity> &entities);
};
