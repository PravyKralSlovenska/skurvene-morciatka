#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/world/world.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/renderer/buffers/vertex_buffer_object.hpp"
#include "engine/renderer/buffers/vertex_array_object.hpp"
#include "engine/renderer/buffers/element_array_object.hpp"
#include "others/utils.hpp"

class World_Renderer
{
private:
    std::unique_ptr<VERTEX_ARRAY_OBJECT> VAO;
    std::unique_ptr<VERTEX_BUFFER_OBJECT> VBO;
    std::unique_ptr<ELEMENT_ARRAY_BUFFER> EBO;
    std::unique_ptr<Shader> shader;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    static constexpr std::array<unsigned int, 6> QUAD_INDICES = {
        0, 1, 2,
        1, 2, 3};

    // std::unique_ptr<VERTEX_ARRAY_OBJECT> chunk_VAO;
    // std::unique_ptr<VERTEX_BUFFER_OBJECT> chunk_VBO;
    // std::unique_ptr<ELEMENT_ARRAY_BUFFER> chunk_EBO;
    // std::unique_ptr<Shader> chunk_shader;

    // std::vector<Vertex> chunk_vertices;
    // std::vector<unsigned int> chunk_indices;

    World *world = nullptr;

public:
    glm::mat4 projection;

public:
    World_Renderer(World *world);
    ~World_Renderer() = default;

    void init();
    void set_world(World *world);
    void set_projection(glm::mat4 projection);

    void render_test_triangle();

    void add_chunk_to_batch(Chunk *chunk);

    void render_chunk_borders();
    void render_world();

    void clear_buffers();

    void fill_vertices();
};
