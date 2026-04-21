#pragma once

// File purpose: Renders entities with batching and texture atlas support.
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>

#include "engine/player/wand.hpp"

// forward declarations
class Entity_Manager;
class Entity;
class Player;
class World;
class Shader;
class VERTEX_ARRAY_OBJECT;
class VERTEX_BUFFER_OBJECT;
class ELEMENT_ARRAY_BUFFER;
struct Chunk_Coords_to_Hash;

// Defines the Texture struct.
struct Texture
{
    unsigned int id = 0;
    int width = 0;
    int height = 0;
    int channels = 0;
};

// Defines the EntityVertex struct.
struct EntityVertex
{
    glm::vec2 position;
    glm::vec2 tex_coords;
    glm::vec4 color;
};

// Batches and renders entities with texture atlases.
class Entities_Renderer
{
private:
    Entity_Manager *entity_manager = nullptr;
    World *world = nullptr;

    // OpenGL objects
    std::unique_ptr<Shader> shader;
    std::unique_ptr<VERTEX_ARRAY_OBJECT> VAO;
    std::unique_ptr<VERTEX_BUFFER_OBJECT> VBO;
    std::unique_ptr<ELEMENT_ARRAY_BUFFER> EBO;

    // textures
    std::unordered_map<std::string, Texture> textures;
    unsigned int default_texture_id = 0;

    // projection
    glm::mat4 projection = glm::mat4(1.0f);

    // batch rendering data
    std::vector<EntityVertex> vertices;
    std::vector<unsigned int> indices;

    // chunk info for filtering
    int chunk_pixel_width = 50;
    int chunk_pixel_height = 50;

private:
    // Sets up buffers.
    void setup_buffers();
    // Creates default texture.
    void create_default_texture();
    // Adds entity to batch.
    void add_entity_to_batch(Entity *entity);

public:
    // Constructs Entities_Renderer.
    Entities_Renderer();
    // Destroys Entities_Renderer and releases owned resources.
    ~Entities_Renderer();

    // Initializes state.
    void init();
    // Sets entity manager.
    void set_entity_manager(Entity_Manager *entity_manager);
    // Sets world.
    void set_world(World *world);
    // Sets projection.
    void set_projection(const glm::mat4 &projection);
    // Sets chunk dimensions.
    void set_chunk_dimensions(int width, int height);

    // texture management
    bool load_texture(const std::string &name, const std::string &path);
    // Binds texture.
    void bind_texture(const std::string &name);
    // Returns texture.
    Texture *get_texture(const std::string &name);

    // rendering
    void render_entities();
    // Renders entities in chunks.
    void render_entities_in_chunks(const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks);
    // Renders entity.
    void render_entity(Entity *entity);

    // Wand rendering
    void render_wand(Player *player);

    // batch rendering
    void begin_batch();
    // End batch.
    void end_batch();
    // Flush.
    void flush();
    // Flush with texture.
    void flush_with_texture(bool use_texture, unsigned int texture_id);

private:
    // Line rendering for wand
    void draw_line(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float thickness = 2.0f);
};
