#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <random>
#include <string>
#include <glm/glm.hpp>

// forward declarations
class Entity;
class Enemy;
class Devushki;
class Boss;
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

// Devushki objective configuration
struct DevushkiObjective
{
    int total_to_collect = 5;    // How many devushki the player needs to collect
    int collected = 0;           // How many have been collected so far
    float collect_range = 50.0f; // Distance at which player "collects" a devushki
    bool objective_active = false;
    bool objective_complete = false;
};

// Sprite sheet configuration for entity types
struct SpriteConfig
{
    std::string path;      // Path to sprite sheet image
    int sheet_width = 128; // Total width of sprite sheet
    int sheet_height = 32; // Total height of sprite sheet
    int frame_width = 32;  // Width of each frame
    int frame_height = 32; // Height of each frame
    int frame_count = 4;   // Number of frames in the sheet
    bool is_valid = false; // Whether this config has been set
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

    // Devushki objective
    DevushkiObjective devushki_objective;
    
    // Deferred devushki spawning on structures
    std::string devushki_sprite_name;  // sprite to use for devushki
    std::unordered_set<int> spawned_devushki_positions; // hash of positions already spawned

    // Sprite registry - stores sprite configs by entity type name
    std::unordered_map<std::string, SpriteConfig> sprite_registry;

private:
    void remove_all_dead();
    void update_entity(Entity *entity, float delta_time);
    void update_spawner(float delta_time);
    void check_and_spawn_devushki_on_structures(); // check for new structures and spawn devushki
    glm::ivec2 get_random_spawn_position();
    glm::ivec2 find_valid_position_for_hitbox(const glm::ivec2 &desired_pos, const glm::ivec2 &hitbox_dims, int max_attempts = 20);
    void randomize_enemy_stats(Enemy *enemy);

public:
    Entity_Manager();
    ~Entity_Manager();

    void set_world(World *world);
    void ensure_player_valid_position();
    void update();
    void update(float delta_time);

    // entity creation - optionally specify a registered sprite name
    Entity *create_entity();
    Enemy *create_enemy(const glm::ivec2 &position, const std::string &sprite_name = "");
    Enemy *create_enemy(int x, int y, const std::string &sprite_name = "");
    Enemy *spawn_random_enemy(const std::string &sprite_name = ""); // spawns with random position & stats

    // devushki creation
    Devushki *create_devushki(const glm::ivec2 &position, const std::string &sprite_name = "");
    Devushki *create_devushki(int x, int y, const std::string &sprite_name = "");

    // devushki objective system
    void spawn_devushki_objective(int count, float spread_radius = 2000.0f, const std::string &sprite_name = "");
    void check_devushki_collection();
    void set_devushki_objective_count(int count);
    void set_devushki_collect_range(float range);
    DevushkiObjective &get_devushki_objective();
    bool is_objective_complete() const;

    // boss creation
    Boss *create_boss(const glm::ivec2 &position, const std::string &sprite_name = "");
    Boss *create_boss(int x, int y, const std::string &sprite_name = "");

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

    // sprite management - register sprites by name for easy reuse
    // Simple registration (uses default 128x32, 4 frames of 32x32)
    void register_sprite(const std::string &name, const std::string &sprite_path);
    // Detailed registration with custom dimensions
    void register_sprite(const std::string &name, const std::string &sprite_path,
                         int sheet_width, int sheet_height, int frame_width, int frame_height, int frame_count);
    // Apply a registered sprite to any entity
    void apply_sprite(Entity *entity, const std::string &name);
    // Query sprites
    bool has_sprite(const std::string &name) const;
    const SpriteConfig *get_sprite_config(const std::string &name) const;
    std::vector<std::string> get_all_sprite_names() const;

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
    int get_devushki_count() const;
    int get_boss_count() const;
    bool has_entity(int id) const;
};
