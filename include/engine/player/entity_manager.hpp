#pragma once

// File purpose: Manages entity lifecycle, spawning, updates, and interactions.
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <random>
#include <string>
#include <cstdint>
#include <glm/glm.hpp>
#include "engine/particle/particle.hpp"
#include "engine/player/entity.hpp"

// forward declarations
class World;
struct Chunk_Coords_to_Hash;
class Structure;

// Enemy spawn configuration
struct SpawnConfig
{
    float min_spawn_distance = 250.0f;  // Min distance from player (50 particles)
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
    bool boss_spawned = false;
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

enum class Store_Offer_Type
{
    HEAL,
    AMMO,
    WAND_SAND,
    WAND_WATER,
    WAND_FIRE,
    WAND_ICE,
    WAND_WATER_VAPOR,
    WAND_WOOD,
    WAND_STONE,
    WAND_EMPTY,
    WAND_SMOKE,
    WAND_URANIUM,
    COMPASS
};

// Defines the Store_Offer struct.
struct Store_Offer
{
    std::int64_t offer_key = 0;
    int structure_hash = 0;
    glm::ivec2 structure_world_pos = {0, 0};
    glm::ivec2 display_world_pos = {0, 0};
    Store_Offer_Type type = Store_Offer_Type::HEAL;
    std::string item_name;
    std::string icon_path;
    int price_gold = 0;
    int price_silver = 0;
    bool purchased = false;
};

// Owns entity lifecycle, updates, combat interactions, and spawns.
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
    std::string devushki_sprite_name;                   // sprite to use for devushki
    std::unordered_set<int> spawned_devushki_positions; // hash of column center positions already used for spawning

    // Sprite registry - stores sprite configs by entity type name
    std::unordered_map<std::string, SpriteConfig> sprite_registry;

    // Currency collected by player
    int collected_gold_coins = 0;
    int collected_silver_coins = 0;

    // Gun ammo economy
    int player_ammo = 60;

    // Special utility store items
    bool player_has_compass = true;

    // Multiple offers per store structure (keyed by offer_key)
    std::unordered_map<std::int64_t, Store_Offer> store_offers_by_structure;

private:
    // Removes all dead.
    void remove_all_dead();
    // Drops enemy coin particles.
    void drop_enemy_coin_particles(const Enemy *enemy);
    // Updates entity.
    void update_entity(Entity *entity, float delta_time);
    // Processes boss special actions.
    void process_boss_special_actions();
    // Resolves projectile entity hits.
    void resolve_projectile_entity_hits();
    // Resolves hostile melee hits.
    void resolve_hostile_melee_hits();
    // Resolves coin collection.
    void resolve_coin_collection();
    // Normalizes currency.
    void normalize_currency();
    // Returns total currency in silver.
    int get_total_currency_in_silver() const;
    // Updates spawner.
    void update_spawner(float delta_time);
    // Updates store offers.
    void update_store_offers();
    // Finds store display anchor.
    bool find_store_display_anchor(const Structure &store_structure, glm::ivec2 &out_anchor_cells) const;
    // Checks and spawn devushki on structures.
    void check_and_spawn_devushki_on_structures(); // check for new structures and spawn devushki
    // Spawns boss for completed objective.
    void spawn_boss_for_completed_objective();
    // Returns random spawn position.
    glm::ivec2 get_random_spawn_position();
    // Finds valid position for hitbox.
    glm::ivec2 find_valid_position_for_hitbox(const glm::ivec2 &desired_pos, const glm::ivec2 &hitbox_dims, int max_attempts = 20);
    // Randomizes enemy stats.
    void randomize_enemy_stats(Enemy *enemy);
    // Returns enemy count in chunks.
    int get_enemy_count_in_chunks(const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks) const;

public:
    // Constructs Entity_Manager.
    Entity_Manager();
    // Destroys Entity_Manager and releases owned resources.
    ~Entity_Manager();

    // Sets world.
    void set_world(World *world);
    // Resets for new world.
    void reset_for_new_world();
    // Ensures player valid position.
    void ensure_player_valid_position();
    // Updates state.
    void update();
    // Updates state.
    void update(float delta_time);

    // entity creation - optionally specify a registered sprite name
    Entity *create_entity();
    // Creates enemy.
    Enemy *create_enemy(const glm::ivec2 &position, const std::string &sprite_name = "");
    // Creates enemy.
    Enemy *create_enemy(int x, int y, const std::string &sprite_name = "");
    // Spawns random enemy.
    Enemy *spawn_random_enemy(const std::string &sprite_name = ""); // spawns with random position & stats

    // devushki creation
    Devushki *create_devushki(const glm::ivec2 &position, const std::string &sprite_name = "");
    // Creates devushki.
    Devushki *create_devushki(int x, int y, const std::string &sprite_name = "");

    // projectile creation (gun shots)
    Projectile *create_projectile(const glm::vec2 &position, const glm::vec2 &velocity,
                                  Particle_Type payload_type = Particle_Type::STONE,
                                  float damage = 25.0f,
                                  Entity_Type owner_type = Entity_Type::PLAYER);

    // devushki objective system
    void spawn_devushki_objective(int count, float spread_radius = 2000.0f, const std::string &sprite_name = "");
    // Checks devushki collection.
    void check_devushki_collection();
    // Sets devushki objective count.
    void set_devushki_objective_count(int count);
    // Sets devushki collect range.
    void set_devushki_collect_range(float range);
    // Returns devushki objective.
    DevushkiObjective &get_devushki_objective();
    // Returns true if objective complete.
    bool is_objective_complete() const;

    // boss creation
    Boss *create_boss(const glm::ivec2 &position, const std::string &sprite_name = "");
    // Creates boss.
    Boss *create_boss(int x, int y, const std::string &sprite_name = "");

    // entity removal
    bool remove_entity(const int id);
    // Removes all entities.
    void remove_all_entities();

    // spawn system control
    void set_spawn_enabled(bool enabled);
    // Sets spawn interval.
    void set_spawn_interval(float interval);
    // Sets max enemies.
    void set_max_enemies(int max);
    // Sets spawn distance.
    void set_spawn_distance(float min_dist, float max_dist);
    // Sets difficulty.
    void set_difficulty(float multiplier);
    // Returns spawn config.
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
    // Returns sprite config.
    const SpriteConfig *get_sprite_config(const std::string &name) const;
    // Returns all sprite names.
    std::vector<std::string> get_all_sprite_names() const;

    // getters
    Player *get_player();
    // Returns entity.
    Entity *get_entity(const int id);
    // Returns all entities.
    std::unordered_map<int, std::unique_ptr<Entity>> *get_all_entities();

    // get entities in specific chunks (for rendering)
    std::vector<Entity *> get_entities_in_chunks(const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks);
    // Returns all active entities.
    std::vector<Entity *> get_all_active_entities();

    // utility
    int get_entity_count() const;
    // Returns enemy count.
    int get_enemy_count() const;
    // Returns devushki count.
    int get_devushki_count() const;
    // Returns boss count.
    int get_boss_count() const;
    // Returns collected gold coins.
    int get_collected_gold_coins() const;
    // Returns collected silver coins.
    int get_collected_silver_coins() const;
    // Returns player ammo.
    int get_player_ammo() const;
    // Returns true if compass.
    bool has_compass() const;
    // Returns nearest devushki position.
    bool get_nearest_devushki_position(glm::ivec2 &out_position, float *out_distance = nullptr) const;
    // Returns compass target position.
    bool get_compass_target_position(glm::ivec2 &out_position, float *out_distance = nullptr) const;
    // Tries consume ammo for shot.
    bool try_consume_ammo_for_shot();
    // Adds player ammo.
    void add_player_ammo(int amount);
    // Returns true if player near store.
    bool is_player_near_store() const;
    // Tries buy store item.
    bool try_buy_store_item();
    // Returns nearest store offer.
    const Store_Offer *get_nearest_store_offer() const;
    // Returns active store offers.
    std::vector<Store_Offer> get_active_store_offers() const;
    // Returns store heal price gold.
    int get_store_heal_price_gold() const;
    // Returns store heal amount.
    int get_store_heal_amount() const;
    // Returns true if entity.
    bool has_entity(int id) const;
};
