#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>

// forward declarations
class Entity_Manager;
class Entity;
class World;
class Shader;
class VERTEX_ARRAY_OBJECT;
class VERTEX_BUFFER_OBJECT;
class ELEMENT_ARRAY_BUFFER;
struct Chunk_Coords_to_Hash;

struct Texture
{
    unsigned int id = 0;
    int width = 0;
    int height = 0;
    int channels = 0;
};

struct EntityVertex
{
    glm::vec2 position;
    glm::vec2 tex_coords;
    glm::vec4 color;
};

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
    void setup_buffers();
    void create_default_texture();
    void add_entity_to_batch(Entity *entity);

public:
    Entities_Renderer();
    ~Entities_Renderer();

    void init();
    void set_entity_manager(Entity_Manager *entity_manager);
    void set_world(World *world);
    void set_projection(const glm::mat4 &projection);
    void set_chunk_dimensions(int width, int height);

    // texture management
    bool load_texture(const std::string &name, const std::string &path);
    void bind_texture(const std::string &name);
    Texture *get_texture(const std::string &name);

    // rendering
    void render_entities();
    void render_entities_in_chunks(const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks);
    void render_entity(Entity *entity);

    // batch rendering
    void begin_batch();
    void end_batch();
    void flush();
};
