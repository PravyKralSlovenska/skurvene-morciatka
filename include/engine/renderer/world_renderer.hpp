#pragma once

// #include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

// #include "engine/world/world.hpp"

// forward declarations
class VERTEX_ARRAY_OBJECT;
class VERTEX_BUFFER_OBJECT;
class ELEMENT_ARRAY_BUFFER;
class Shader;
class Compute_Shader;
class World;
class Chunk;
class Vertex;
class Shader_Storage_Buffer_Object;

// struct GPU_particle
// {
// };

// struct GPU_chunk
// {
// };

class World_Renderer
{
private:
    std::unique_ptr<VERTEX_ARRAY_OBJECT> dummy_VAO;

    std::unique_ptr<Shader> render_shader;

    std::unique_ptr<Compute_Shader> compute_shader;
    std::unique_ptr<Shader_Storage_Buffer_Object> particle_ssbo;
    std::unique_ptr<Shader_Storage_Buffer_Object> vertex_ssbo;
    std::unique_ptr<Shader_Storage_Buffer_Object> chunk_ssbo;
    std::unique_ptr<Shader_Storage_Buffer_Object> vertex_counter_ssbo;

    World *world = nullptr;

public:
    glm::mat4 projection;

public:
    World_Renderer(World *world);
    ~World_Renderer();

    void init();
    void set_world(World *world);
    void set_projection(glm::mat4 projection);

    void render_test_triangle();

    void add_chunk_to_batch(Chunk *chunk);

    // nove
    void render_world_compute();

    // stare
    void render_chunk_borders();
    void render_world();

    void clear_buffers();

    void fill_vertices();
};
