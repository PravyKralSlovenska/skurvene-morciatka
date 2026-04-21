#pragma once

// File purpose: Renders world chunks and GPU-side world data.
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
// Renders chunk/world geometry and GPU world data.

class World_Renderer
{
private:
    static constexpr float FOG_START_CHUNK_FACTOR = 12.0f;
    static constexpr float FOG_END_CHUNK_FACTOR = 14.5f;

    std::unique_ptr<VERTEX_ARRAY_OBJECT> dummy_VAO;

    std::unique_ptr<Shader> render_shader;

    std::unique_ptr<Compute_Shader> compute_shader;
    std::unique_ptr<Shader_Storage_Buffer_Object> particle_ssbo;
    std::unique_ptr<Shader_Storage_Buffer_Object> vertex_ssbo;
    std::unique_ptr<Shader_Storage_Buffer_Object> chunk_ssbo;
    std::unique_ptr<Shader_Storage_Buffer_Object> vertex_counter_ssbo;

    World *world = nullptr;
    // 2D vector helper.
    glm::vec2 fog_center_world = glm::vec2(0.0f, 0.0f);
    bool fog_enabled = true;

public:
    glm::mat4 projection;

public:
    // Constructs World_Renderer.
    World_Renderer(World *world);
    // Destroys World_Renderer and releases owned resources.
    ~World_Renderer();

    // Initializes state.
    void init();
    // Sets world.
    void set_world(World *world);
    // Sets projection.
    void set_projection(glm::mat4 projection);
    // Sets fog center world.
    void set_fog_center_world(const glm::vec2 &world_pos);
    // Sets fog enabled.
    void set_fog_enabled(bool enabled);

    // Renders test triangle.
    void render_test_triangle();

    // Adds chunk to batch.
    void add_chunk_to_batch(Chunk *chunk);

    // nove
    void render_world_compute();

    // stare
    void render_chunk_borders();
    // Renders world.
    void render_world();

    // Clears buffers.
    void clear_buffers();

    // Fills vertices.
    void fill_vertices();
};
