#include "engine/renderer/entities_renderer.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "engine/player/entity_manager.hpp"
#include "engine/player/entity.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/renderer/buffers/vertex_array_object.hpp"
#include "engine/renderer/buffers/vertex_buffer_object.hpp"
#include "engine/renderer/buffers/element_array_object.hpp"
#include "others/GLOBALS.hpp"

// stb_image implementation is in herringbone_world_generation.cpp
// just include the header here
#include "stb/stb_image.h"

Entities_Renderer::Entities_Renderer() {}

Entities_Renderer::~Entities_Renderer()
{
    // Delete textures
    for (auto &[name, texture] : textures)
    {
        if (texture.id != 0)
        {
            glDeleteTextures(1, &texture.id);
        }
    }

    if (default_texture_id != 0)
    {
        glDeleteTextures(1, &default_texture_id);
    }
}

void Entities_Renderer::init()
{
    // Create shader - using same basic shaders as world
    shader = std::make_unique<Shader>("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    shader->create_shader();

    // Setup buffers
    setup_buffers();

    // Create a default white texture for entities without textures
    create_default_texture();
}

void Entities_Renderer::setup_buffers()
{
    VAO = std::make_unique<VERTEX_ARRAY_OBJECT>();
    VBO = std::make_unique<VERTEX_BUFFER_OBJECT>();
    EBO = std::make_unique<ELEMENT_ARRAY_BUFFER>();

    VAO->bind();
    VBO->bind();

    // Setup vertex attributes for EntityVertex: position (vec2), tex_coords (vec2), color (vec4)
    // Position attribute
    VAO->setup_vertex_attribute_pointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void *)0);
    // Texture coords attribute (not used for now, but reserved)
    VAO->setup_vertex_attribute_pointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void *)(2 * sizeof(float)));
    // Color attribute
    VAO->setup_vertex_attribute_pointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void *)(4 * sizeof(float)));

    VBO->unbind();
    VAO->unbind();
}

void Entities_Renderer::create_default_texture()
{
    // Create a simple 1x1 white texture
    unsigned char white_pixel[] = {255, 255, 255, 255};

    glGenTextures(1, &default_texture_id);
    glBindTexture(GL_TEXTURE_2D, default_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Entities_Renderer::set_entity_manager(Entity_Manager *entity_manager)
{
    this->entity_manager = entity_manager;
}

void Entities_Renderer::set_world(World *world)
{
    this->world = world;

    if (world)
    {
        glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        chunk_pixel_width = chunk_dims.x * static_cast<int>(Globals::PARTICLE_SIZE);
        chunk_pixel_height = chunk_dims.y * static_cast<int>(Globals::PARTICLE_SIZE);
    }
}

void Entities_Renderer::set_projection(const glm::mat4 &proj)
{
    projection = proj;
}

void Entities_Renderer::set_chunk_dimensions(int width, int height)
{
    chunk_pixel_width = width;
    chunk_pixel_height = height;
}

bool Entities_Renderer::load_texture(const std::string &name, const std::string &path)
{
    // Check if already loaded
    if (textures.find(name) != textures.end())
    {
        return true;
    }

    Texture texture;

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &texture.width, &texture.height, &texture.channels, 4);

    if (!data)
    {
        std::cerr << "[Entities_Renderer] Failed to load texture: " << path << '\n';
        return false;
    }

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    textures[name] = texture;

    std::cout << "[Entities_Renderer] Loaded texture: " << name << " (" << texture.width << "x" << texture.height << ")\n";

    return true;
}

void Entities_Renderer::bind_texture(const std::string &name)
{
    auto it = textures.find(name);
    if (it != textures.end())
    {
        glBindTexture(GL_TEXTURE_2D, it->second.id);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, default_texture_id);
    }
}

Texture *Entities_Renderer::get_texture(const std::string &name)
{
    auto it = textures.find(name);
    if (it != textures.end())
    {
        return &it->second;
    }
    return nullptr;
}

void Entities_Renderer::begin_batch()
{
    vertices.clear();
    indices.clear();
}

void Entities_Renderer::add_entity_to_batch(Entity *entity)
{
    if (!entity || !entity->is_active)
        return;

    // Get entity position and dimensions
    glm::vec2 pos = glm::vec2(entity->coords);
    glm::vec2 half_size = glm::vec2(entity->hitbox_dimensions_half);

    // Determine color based on entity type
    glm::vec4 color;
    switch (entity->type)
    {
    case Entity_Type::PLAYER:
        color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for player
        break;
    case Entity_Type::ENEMY:
        color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for enemy
        break;
    case Entity_Type::NPC:
        color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // Blue for NPC
        break;
    case Entity_Type::PROJECTILE:
        color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for projectile
        break;
    default:
        color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // White default
        break;
    }

    // If entity is dead, make it darker
    if (!entity->is_alive)
    {
        color *= 0.3f;
        color.a = 0.5f;
    }

    // Calculate quad vertices
    unsigned int base_index = static_cast<unsigned int>(vertices.size());

    // Top-left
    vertices.push_back({{pos.x - half_size.x, pos.y - half_size.y},
                        {0.0f, 1.0f},
                        color});

    // Top-right
    vertices.push_back({{pos.x + half_size.x, pos.y - half_size.y},
                        {1.0f, 1.0f},
                        color});

    // Bottom-right
    vertices.push_back({{pos.x + half_size.x, pos.y + half_size.y},
                        {1.0f, 0.0f},
                        color});

    // Bottom-left
    vertices.push_back({{pos.x - half_size.x, pos.y + half_size.y},
                        {0.0f, 0.0f},
                        color});

    // Add indices for two triangles
    indices.push_back(base_index + 0);
    indices.push_back(base_index + 1);
    indices.push_back(base_index + 2);

    indices.push_back(base_index + 0);
    indices.push_back(base_index + 2);
    indices.push_back(base_index + 3);
}

void Entities_Renderer::flush()
{
    if (vertices.empty())
        return;

    shader->use();
    shader->set_mat4("projection", projection);

    VAO->bind();

    // Update VBO with vertex data
    VBO->bind();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(EntityVertex), vertices.data(), GL_DYNAMIC_DRAW);

    // Update EBO with index data
    EBO->bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    // Draw
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    VAO->unbind();
}

void Entities_Renderer::end_batch()
{
    flush();
}

void Entities_Renderer::render_entity(Entity *entity)
{
    begin_batch();
    add_entity_to_batch(entity);
    end_batch();
}

void Entities_Renderer::render_entities()
{
    if (!entity_manager)
        return;

    begin_batch();

    // Add player to batch
    Player *player = entity_manager->get_player();
    if (player)
    {
        add_entity_to_batch(player);
    }

    // Add all other entities
    auto entities = entity_manager->get_all_active_entities();
    for (Entity *entity : entities)
    {
        add_entity_to_batch(entity);
    }

    end_batch();
}

void Entities_Renderer::render_entities_in_chunks(
    const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks)
{
    if (!entity_manager || !world)
        return;

    begin_batch();

    // Always render player
    Player *player = entity_manager->get_player();
    if (player)
    {
        add_entity_to_batch(player);
    }

    // Get entities that are in active chunks
    auto entities = entity_manager->get_entities_in_chunks(active_chunks);
    for (Entity *entity : entities)
    {
        add_entity_to_batch(entity);
    }

    end_batch();
}
