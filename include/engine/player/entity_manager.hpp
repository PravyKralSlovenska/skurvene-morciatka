#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <random>
#include <glm/glm.hpp>

// forward declarations
class Entity;
class Enemy;
class Player;
class World;
struct Chunk_Coords_to_Hash;

// Enemy spawn configuration
struct SpawnConfig
{
    float min_spawn_distance = 200.0f;  // Min distance from player
    float max_spawn_distance = 500.0f;  // Max distance from player
    float spawn_interval = 3.0f;        // Seconds between spawn attempts
    int max_enemies = 20;               // Maximum enemies at once
    float difficulty_multiplier = 1.0f; // Affects enemy stats
    bool spawn_enabled = true;
};

class Entity_Manager
{
private:
    std::unique_ptr<Player> player;
    std::unordered_map<int, std::unique_ptr<Entity>> entities;
    World *world = nullptr;

    // chunk tracking for entities
    int chunk_pixel_width = 50; // will be set from world
    int chunk_pixel_height = 50;

    // Spawn system
    SpawnConfig spawn_config;
    float spawn_timer = 0.0f;
    std::mt19937 rng;

private:
    void remove_all_dead();
    void update_entity(Entity *entity, float delta_time);
    void update_spawner(float delta_time);
    glm::ivec2 get_random_spawn_position();
    void randomize_enemy_stats(Enemy *enemy);

public:
    Entity_Manager();
    ~Entity_Manager();

    void set_world(World *world);
    void update();
    void update(float delta_time);

    // entity creation
    Entity *create_entity();
    Enemy *create_enemy(const glm::ivec2 &position);
    Enemy *create_enemy(int x, int y);
    Enemy *spawn_random_enemy(); // New: spawns with random position & stats

    // entity removal
    bool remove_entity(const int id);
    void remove_all_entities();

    // spawn system control
    void set_spawn_enabled(bool enabled);
    void set_spawn_interval(float interval);
    void set_max_enemies(int max);
    void set_spawn_distance(float min_dist, float max_dist);
    void set_difficulty(float multiplier);
    SpawnConfig &get_spawn_config();

    // getters
    Player *get_player();
    Entity *get_entity(const int id);
    std::unordered_map<int, std::unique_ptr<Entity>> *get_all_entities();

    // get entities in specific chunks (for rendering)
    std::vector<Entity *> get_entities_in_chunks(const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks);
    std::vector<Entity *> get_all_active_entities();

    // utility
    int get_entity_count() const;
    int get_enemy_count() const;
    bool has_entity(int id) const;
};
