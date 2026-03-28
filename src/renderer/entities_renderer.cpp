#include "engine/renderer/entities_renderer.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

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
    // Create shader - using entity-specific shaders with texture support
    shader = std::make_unique<Shader>("../shaders/entity/entity_vertex.glsl", "../shaders/entity/entity_fragment.glsl");
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
        color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow default for projectile
        if (const Projectile *projectile = static_cast<const Projectile *>(entity))
        {
            if (projectile->get_payload_type() == Particle_Type::FIRE)
            {
                color = glm::vec4(1.0f, 0.35f, 0.05f, 1.0f); // Hot orange for fireballs
            }
        }
        break;
    case Entity_Type::DEVUSHKI:
        color = glm::vec4(1.0f, 0.5f, 0.8f, 1.0f); // Pink for devushki
        break;
    case Entity_Type::BOSS:
        color = glm::vec4(0.6f, 0.0f, 0.6f, 1.0f); // Purple for boss
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

    // Get UV coordinates - either from sprite animation or default full texture
    glm::vec2 uv_min = {0.0f, 0.0f};
    glm::vec2 uv_max = {1.0f, 1.0f};

    if (entity->has_sprite_animation())
    {
        // Get UV coordinates from sprite animation
        Sprite_Animation &anim = entity->get_sprite_animation();
        uv_min = anim.get_uv_min();
        uv_max = anim.get_uv_max();

        // When using a texture, use white color to show true texture colors
        color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        // If entity is dead, tint it darker
        if (!entity->is_alive)
        {
            color = glm::vec4(0.3f, 0.3f, 0.3f, 0.5f);
        }
    }

    // Calculate quad vertices
    unsigned int base_index = static_cast<unsigned int>(vertices.size());

    // Note: UV Y is flipped because OpenGL textures are usually loaded with Y pointing up
    // Top-left
    vertices.push_back({{pos.x - half_size.x, pos.y - half_size.y},
                        {uv_min.x, uv_max.y},
                        color});

    // Top-right
    vertices.push_back({{pos.x + half_size.x, pos.y - half_size.y},
                        {uv_max.x, uv_max.y},
                        color});

    // Bottom-right
    vertices.push_back({{pos.x + half_size.x, pos.y + half_size.y},
                        {uv_max.x, uv_min.y},
                        color});

    // Bottom-left
    vertices.push_back({{pos.x - half_size.x, pos.y + half_size.y},
                        {uv_min.x, uv_min.y},
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
    // Default flush with no texture
    flush_with_texture(false, default_texture_id);
}

void Entities_Renderer::flush_with_texture(bool use_texture, unsigned int texture_id)
{
    if (vertices.empty())
        return;

    shader->use();
    shader->set_mat4("projection", projection);
    shader->set_bool("useTexture", use_texture);
    shader->set_int("entityTexture", 0);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

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
    glBindTexture(GL_TEXTURE_2D, 0);
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

    // Separate entities into textured and non-textured groups
    std::vector<Entity *> non_textured_entities;
    std::vector<Entity *> textured_entities;

    // Always render player
    Player *player = entity_manager->get_player();
    if (player)
    {
        if (player->has_sprite_animation())
            textured_entities.push_back(player);
        else
            non_textured_entities.push_back(player);
    }

    // Get entities that are in active chunks and sort them
    auto entities = entity_manager->get_entities_in_chunks(active_chunks);
    for (Entity *entity : entities)
    {
        if (entity->has_sprite_animation())
            textured_entities.push_back(entity);
        else
            non_textured_entities.push_back(entity);
    }

    // First, render all non-textured entities in one batch
    if (!non_textured_entities.empty())
    {
        begin_batch();
        for (Entity *entity : non_textured_entities)
        {
            add_entity_to_batch(entity);
        }
        flush_with_texture(false, 0);
        vertices.clear();
        indices.clear();
    }

    // Then, render textured entities - group by texture path
    std::unordered_map<std::string, std::vector<Entity *>> entities_by_texture;
    for (Entity *entity : textured_entities)
    {
        const std::string &path = entity->get_sprite_animation().get_sprite_path();
        entities_by_texture[path].push_back(entity);
    }

    for (auto &[texture_path, ents] : entities_by_texture)
    {
        // Load texture if not already loaded
        if (textures.find(texture_path) == textures.end())
        {
            load_texture(texture_path, texture_path);
        }

        begin_batch();
        for (Entity *entity : ents)
        {
            add_entity_to_batch(entity);
        }

        // Get texture ID
        unsigned int tex_id = default_texture_id;
        auto it = textures.find(texture_path);
        if (it != textures.end())
        {
            tex_id = it->second.id;
        }

        flush_with_texture(true, tex_id);
        vertices.clear();
        indices.clear();
    }

    // Render wand after entities
    if (player)
    {
        render_wand(player);
    }
}

void Entities_Renderer::draw_line(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float thickness)
{
    // Calculate perpendicular direction for line thickness
    glm::vec2 dir = end - start;
    float length = glm::length(dir);
    if (length < 0.001f)
        return;

    dir = dir / length; // normalize
    glm::vec2 perp = glm::vec2(-dir.y, dir.x) * (thickness * 0.5f);

    // Create quad vertices for the line
    unsigned int base_index = static_cast<unsigned int>(vertices.size());

    // Four corners of the line quad
    vertices.push_back({{start.x + perp.x, start.y + perp.y}, {0.0f, 0.0f}, color});
    vertices.push_back({{start.x - perp.x, start.y - perp.y}, {0.0f, 1.0f}, color});
    vertices.push_back({{end.x - perp.x, end.y - perp.y}, {1.0f, 1.0f}, color});
    vertices.push_back({{end.x + perp.x, end.y + perp.y}, {1.0f, 0.0f}, color});

    // Two triangles
    indices.push_back(base_index + 0);
    indices.push_back(base_index + 1);
    indices.push_back(base_index + 2);

    indices.push_back(base_index + 0);
    indices.push_back(base_index + 2);
    indices.push_back(base_index + 3);
}

void Entities_Renderer::render_wand(Player *player)
{
    if (!player)
        return;

    const Wand &wand = player->get_current_wand();
    if (wand.is_empty())
        return;

    // Get player center position
    glm::vec2 player_center = player->get_center();

    // Get cursor position (target)
    glm::vec2 cursor_pos = player->get_cursor_world_pos();

    // Calculate direction from player to cursor
    glm::vec2 direction = cursor_pos - player_center;
    float distance = glm::length(direction);

    // Limit wand visual length
    const float WAND_LENGTH = 40.0f; // Visual length of the wand line

    glm::vec2 wand_end;
    if (distance > 0.001f)
    {
        glm::vec2 normalized_dir = direction / distance;
        wand_end = player_center + normalized_dir * WAND_LENGTH;
    }
    else
    {
        // Default direction (right) if cursor is on player
        wand_end = player_center + glm::vec2(WAND_LENGTH, 0.0f);
    }

    // Draw the wand line
    begin_batch();
    draw_line(player_center, wand_end, wand.color, 3.0f);
    end_batch();
}
