#include "engine/player/entity_manager.hpp"

#include <cmath>
#include "engine/player/entity.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "others/GLOBALS.hpp"

Entity_Manager::Entity_Manager()
    : rng(std::random_device{}())
{
    player = std::make_unique<Player>("Player", glm::vec2(0, 0));
}

Entity_Manager::~Entity_Manager() = default;

void Entity_Manager::set_world(World *world)
{
    this->world = world;

    if (world)
    {
        glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        chunk_pixel_width = chunk_dims.x * static_cast<int>(Globals::PARTICLE_SIZE);
        chunk_pixel_height = chunk_dims.y * static_cast<int>(Globals::PARTICLE_SIZE);

        // Set world reference on player
        if (player)
        {
            player->set_world(world);
        }

        // Set world reference on all existing entities
        for (auto &[id, entity] : entities)
        {
            entity->set_world(world);
        }
    }
}

Entity *Entity_Manager::create_entity()
{
    auto entity = std::make_unique<Entity>();
    int id = entity->get_id();
    entities[id] = std::move(entity);
    return entities[id].get();
}

Enemy *Entity_Manager::create_enemy(const glm::ivec2 &position)
{
    auto enemy = std::make_unique<Enemy>();
    enemy->set_position(position);

    // Set world reference for collision detection
    if (world)
    {
        enemy->set_world(world);
    }

    int id = enemy->get_id();
    Enemy *enemy_ptr = enemy.get();
    entities[id] = std::move(enemy);

    return enemy_ptr;
}

Enemy *Entity_Manager::create_enemy(int x, int y)
{
    return create_enemy(glm::ivec2(x, y));
}

bool Entity_Manager::remove_entity(const int id)
{
    return entities.erase(id) > 0;
}

void Entity_Manager::remove_all_entities()
{
    entities.clear();
}

void Entity_Manager::remove_all_dead()
{
    for (auto it = entities.begin(); it != entities.end();)
    {
        if (!it->second->get_is_alive())
        {
            it = entities.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Entity_Manager::update_entity(Entity *entity, float delta_time)
{
    if (!entity || !entity->is_active)
        return;

    entity->update(delta_time);

    // If it's an enemy, update its target to player position
    if (entity->type == Entity_Type::ENEMY && player)
    {
        Enemy *enemy = static_cast<Enemy *>(entity);
        enemy->set_target(player->coords);
    }
}

void Entity_Manager::update()
{
    update(1.0f / 60.0f); // Default to ~60 FPS delta
}

void Entity_Manager::update(float delta_time)
{
    // Update spawner
    update_spawner(delta_time);

    // Update player physics (gravity, collision) - input is handled separately
    if (player)
    {
        player->update(delta_time);
    }

    // Update all entities (enemies, NPCs, etc.)
    for (auto &[id, entity] : entities)
    {
        update_entity(entity.get(), delta_time);
    }

    // Remove dead entities
    remove_all_dead();
}

// ==================== Spawn System ====================

void Entity_Manager::update_spawner(float delta_time)
{
    if (!spawn_config.spawn_enabled)
        return;

    spawn_timer += delta_time;

    if (spawn_timer >= spawn_config.spawn_interval)
    {
        spawn_timer = 0.0f;

        // Check if we can spawn more enemies
        if (get_enemy_count() < spawn_config.max_enemies)
        {
            spawn_random_enemy();
        }
    }
}

glm::ivec2 Entity_Manager::get_random_spawn_position()
{
    if (!player)
        return {0, 0};

    // Generate random angle
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * 3.14159265f);
    float angle = angle_dist(rng);

    // Generate random distance between min and max
    std::uniform_real_distribution<float> dist_dist(
        spawn_config.min_spawn_distance,
        spawn_config.max_spawn_distance);
    float distance = dist_dist(rng);

    // Calculate spawn position
    glm::ivec2 spawn_pos;
    spawn_pos.x = player->coords.x + static_cast<int>(std::cos(angle) * distance);
    spawn_pos.y = player->coords.y + static_cast<int>(std::sin(angle) * distance);

    return spawn_pos;
}

void Entity_Manager::randomize_enemy_stats(Enemy *enemy)
{
    if (!enemy)
        return;

    float diff = spawn_config.difficulty_multiplier;

    // Randomize health (40-60 base, scaled by difficulty)
    std::uniform_real_distribution<float> health_dist(40.0f, 60.0f);
    float health = health_dist(rng) * diff;
    enemy->healthpoints = health;
    enemy->max_healthpoints = health;

    // Randomize speed (40-70 base)
    std::uniform_real_distribution<float> speed_dist(40.0f, 70.0f);
    enemy->speed = speed_dist(rng);

    // Randomize detection range (150-300)
    std::uniform_real_distribution<float> detect_dist(150.0f, 300.0f);
    enemy->set_detection_range(detect_dist(rng));

    // Randomize attack damage (8-15, scaled by difficulty)
    std::uniform_real_distribution<float> damage_dist(8.0f, 15.0f);
    enemy->set_attack_damage(damage_dist(rng) * diff);

    // Randomize attack range (30-60)
    std::uniform_real_distribution<float> range_dist(30.0f, 60.0f);
    enemy->set_attack_range(range_dist(rng));

    // Random chance to add patrol points
    std::uniform_int_distribution<int> patrol_chance(0, 3);
    if (patrol_chance(rng) == 0) // 25% chance
    {
        glm::ivec2 home = enemy->coords;
        std::uniform_int_distribution<int> offset_dist(-100, 100);

        int num_points = 2 + (rng() % 3); // 2-4 patrol points
        for (int i = 0; i < num_points; i++)
        {
            enemy->add_patrol_point({home.x + offset_dist(rng),
                                     home.y + offset_dist(rng)});
        }
    }
}

Enemy *Entity_Manager::spawn_random_enemy()
{
    glm::ivec2 spawn_pos = get_random_spawn_position();
    Enemy *enemy = create_enemy(spawn_pos);

    if (enemy)
    {
        enemy->set_home_position(spawn_pos);
        randomize_enemy_stats(enemy);
    }

    return enemy;
}

// ==================== Spawn Config Setters ====================

void Entity_Manager::set_spawn_enabled(bool enabled)
{
    spawn_config.spawn_enabled = enabled;
}

void Entity_Manager::set_spawn_interval(float interval)
{
    spawn_config.spawn_interval = interval;
}

void Entity_Manager::set_max_enemies(int max)
{
    spawn_config.max_enemies = max;
}

void Entity_Manager::set_spawn_distance(float min_dist, float max_dist)
{
    spawn_config.min_spawn_distance = min_dist;
    spawn_config.max_spawn_distance = max_dist;
}

void Entity_Manager::set_difficulty(float multiplier)
{
    spawn_config.difficulty_multiplier = multiplier;
}

SpawnConfig &Entity_Manager::get_spawn_config()
{
    return spawn_config;
}

Player *Entity_Manager::get_player()
{
    return player.get();
}

Entity *Entity_Manager::get_entity(const int id)
{
    auto it = entities.find(id);
    if (it != entities.end())
    {
        return it->second.get();
    }
    return nullptr;
}

std::unordered_map<int, std::unique_ptr<Entity>> *Entity_Manager::get_all_entities()
{
    return &entities;
}

std::vector<Entity *> Entity_Manager::get_entities_in_chunks(
    const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks)
{
    std::vector<Entity *> result;

    for (auto &[id, entity] : entities)
    {
        if (!entity->is_active)
            continue;

        glm::ivec2 entity_chunk = entity->get_chunk_position(chunk_pixel_width, chunk_pixel_height);

        if (active_chunks.find(entity_chunk) != active_chunks.end())
        {
            result.push_back(entity.get());
        }
    }

    return result;
}

std::vector<Entity *> Entity_Manager::get_all_active_entities()
{
    std::vector<Entity *> result;

    for (auto &[id, entity] : entities)
    {
        if (entity->is_active)
        {
            result.push_back(entity.get());
        }
    }

    return result;
}

int Entity_Manager::get_entity_count() const
{
    return static_cast<int>(entities.size());
}

int Entity_Manager::get_enemy_count() const
{
    int count = 0;
    for (const auto &[id, entity] : entities)
    {
        if (entity->type == Entity_Type::ENEMY && entity->is_active)
        {
            count++;
        }
    }
    return count;
}

bool Entity_Manager::has_entity(int id) const
{
    return entities.find(id) != entities.end();
}