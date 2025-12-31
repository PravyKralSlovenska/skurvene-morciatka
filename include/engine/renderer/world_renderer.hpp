#pragma once

// #include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "engine/world/world.hpp" 

// forward declarations
class VERTEX_ARRAY_OBJECT;
class VERTEX_BUFFER_OBJECT;
class ELEMENT_ARRAY_BUFFER;
class Shader;
class Compute_Shader;
class World;
class Chunk;
class Vertex;

// GPU-side particle data (must match compute shader struct!)
struct GPU_Particle
{
    uint32_t type;  // Particle_Type
    uint32_t state; // Particle_State
    uint32_t _pad1; // Padding for alignment
    uint32_t _pad2;
    float color[4]; // RGBA
};

// GPU chunk management
struct GPU_Chunk_Data
{
    unsigned int ssbo_particles; // Buffer holding particle data
    unsigned int ssbo_vertices;  // Buffer holding generated vertices
    unsigned int vao;            // VAO for rendering this chunk
    unsigned int ebo;            // VAO for rendering this chunk
    size_t particle_count;       // How many particles in this chunk
    bool dirty;                  // Does this chunk need updating?
};

class World_Renderer
{
private:
    std::unique_ptr<VERTEX_ARRAY_OBJECT> VAO;
    std::unique_ptr<VERTEX_BUFFER_OBJECT> VBO;
    std::unique_ptr<ELEMENT_ARRAY_BUFFER> EBO;
    std::unique_ptr<Shader> shader;

    std::unique_ptr<Compute_Shader> compute_shader;
    // std::unordered_map<glm::ivec2, GPU_Chunk_Data> gpu_chunks;
    std::unordered_map<glm::ivec2, GPU_Chunk_Data, Chunk_Coords_to_Hash> gpu_chunks;

    // std::vector<Vertex> vertices;
    // std::vector<unsigned int> indices;

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

private:
    void upload_chunk_to_gpu(const glm::ivec2 &chunk_coords, Chunk *chunk);
    void cleanup_chunk_gpu_data(const glm::ivec2 &chunk_coords);

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
