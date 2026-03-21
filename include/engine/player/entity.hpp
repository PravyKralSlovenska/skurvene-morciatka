#pragma once

#include <iostream>
#include <array>
#include <functional>
#include <vector>
#include <glm/glm.hpp>

#include "engine/player/inventory.hpp"
#include "engine/player/wand.hpp"
#include "engine/player/sprite_animation.hpp"

// forward declarations
class Item;
class World;

enum class Entity_States
{
    STILL,
    WALKING,
    JUMPING,
    FALLING,
    HIT,
    DEAD
};

enum class Entity_Type
{
    PLAYER,
    ENEMY,
    NPC,
    PROJECTILE,
    DEVUSHKI,
    BOSS
};

// FSM AI States for enemies
enum class AI_State
{
    IDLE,   // Standing still, looking around
    PATROL, // Walking between patrol points
    CHASE,  // Actively pursuing target
    ATTACK, // In attack range, attacking
    FLEE,   // Running away (low health)
    RETURN, // Returning to patrol/home position
    DEAD    // Dead, no behavior
};

class Entity
{
protected:
    static int next_id;

public:
    int ID;
    Entity_Type type = Entity_Type::ENEMY;
    Entity_States state = Entity_States::STILL;
    bool is_alive = true;
    bool is_active = true;
    bool noclip = false;         // Can move through solid terrain
    bool on_ground = false;      // Is standing on solid ground
    bool gravity_enabled = true; // Apply gravity to this entity

    int size = 1;
    int sprite = 0; // konkretne ktory snimok je aktualny
    float speed = 10.0f;
    float max_healthpoints = 100.0f;
    float healthpoints = 100.0f;
    float damage_invuln_duration = 0.15f;
    float damage_invuln_timer = 0.0f;
    float damage_flash_duration = 0.12f;
    float damage_flash_timer = 0.0f;
    glm::vec2 velocity = {0.0f, 0.0f};
    glm::vec2 acceleration = {0.0f, 0.0f};

    // Physics constants
    static constexpr float GRAVITY = 500.0f;   // pixels per second^2
    static constexpr int MAX_STEP_HEIGHT = 10; // max particles entity can climb

    glm::ivec2 coords = {0, 0};              // stred hitboxu, sledujeho kamera
    glm::ivec2 hitbox_dimensions = {32, 32}; // sirka a vyska
    glm::ivec2 hitbox_dimensions_half = {16, 16};
    glm::ivec2 hitbox[4]; // konretne 4 body vo svete, ktore tvoria hranice hitboxu

    std::string entity_sprite; // kazdy entity sprite bude rovnako velky

    // Sprite animation system
    Sprite_Animation sprite_animation;
    bool use_sprite_animation = false; // whether to use sprite sheet or simple colored quad

    // World reference for collision
    World *world_ref = nullptr;

public:
    Entity();
    virtual ~Entity() = default;

    // update - called every frame for AI/physics
    virtual void update(float delta_time);
    virtual void update_physics(float delta_time);

    // setters
    void set_hitbox_dimensions(const int width, const int height);
    void set_hitbox_dimensions(const glm::ivec2 &hitbox_dimensions);

    void set_position(const int x, const int y);
    void set_position(const glm::ivec2 &position);

    void set_sprite_file(const std::string &path);
    void setup_sprite_sheet(const std::string &path, int sheet_width, int sheet_height,
                            int frame_width, int frame_height, int num_frames);
    void update_sprite_state(); // updates sprite based on entity state
    Sprite_Animation &get_sprite_animation();
    bool has_sprite_animation() const;
    void set_velocity(float vx, float vy);
    void set_velocity(const glm::vec2 &vel);
    void set_world(World *world);
    void set_noclip(bool enabled);
    void toggle_noclip();

    // nemozem to dat private
    void calculate_hitbox();

    // getters
    int get_id() const;
    bool get_is_alive() const;
    Entity_States get_state() const;
    bool get_noclip() const;
    glm::ivec2 get_chunk_position(int chunk_pixel_width, int chunk_pixel_height) const;

    // collision detection
    bool check_collision_at(const glm::ivec2 &position) const;
    bool is_solid_at(int world_x, int world_y) const;
    int get_ground_height_at(int world_x, int start_y, int max_check) const;
    bool can_move_to(const glm::ivec2 &new_pos) const;
    void resolve_collision(glm::ivec2 &new_pos);

    // spawn validation - finds empty space for entity's hitbox
    bool is_valid_spawn_position(const glm::ivec2 &position) const;
    glm::ivec2 find_valid_spawn_position(const glm::ivec2 &desired_pos, int max_search_radius = 500) const;

    // health/damage
    void take_damage(float damage);
    void heal(float amount);
    void die();
    void update_damage_timers(float delta_time);
    bool can_take_damage() const;

    // actions
    void select_item();
    void shoot();

    // moving
    void move_up();
    void move_down();
    void move_left();
    void move_right();
    void jump();

    void go_to(const glm::ivec2 &coords);
    void move_to(const glm::ivec2 &coords);
    void move_by(float dx, float dy);

    // physics
    void apply_gravity(float gravity, float delta_time);
    void apply_velocity(float delta_time);
};

class Player : public Entity
{
private:
    Inventory inventory;
    Hotbar hotbar;

    // Wand aiming
    glm::vec2 aim_direction = {1.0f, 0.0f};    // Direction player is aiming
    glm::vec2 cursor_world_pos = {0.0f, 0.0f}; // Cursor position in world coords

public:
    std::string name;
    int selected_item = 0;

public:
    Player(std::string name, glm::vec2 coords);
    ~Player() = default;

    void update(float delta_time) override;

    void change_selected_item(const int inventory_slot);
    void change_selected_item(const Item item);

    // Hotbar/Wand methods
    Hotbar &get_hotbar() { return hotbar; }
    const Hotbar &get_hotbar() const { return hotbar; }
    void select_hotbar_slot(int slot) { hotbar.select_slot(slot); }
    Wand &get_current_wand() { return hotbar.get_selected_wand(); }

    // Aiming
    void set_aim_direction(const glm::vec2 &dir) { aim_direction = glm::normalize(dir); }
    glm::vec2 get_aim_direction() const { return aim_direction; }
    void set_cursor_world_pos(const glm::vec2 &pos) { cursor_world_pos = pos; }
    glm::vec2 get_cursor_world_pos() const { return cursor_world_pos; }

    // Get center position (for wand origin)
    glm::vec2 get_center() const { return glm::vec2(coords); }
};

class Projectile : public Entity
{
private:
    Particle_Type payload_type = Particle_Type::STONE;
    float damage = 25.0f;
    Entity_Type owner_type = Entity_Type::PLAYER;
    float lifetime_seconds = 3.0f;
    float age_seconds = 0.0f;
    float gravity_multiplier = 0.3f;
    float air_drag = 0.995f;

public:
    Projectile();
    Projectile(const glm::vec2 &position, const glm::vec2 &velocity, Particle_Type payload_type);
    ~Projectile() = default;

    void update(float delta_time) override;

    void set_payload_type(Particle_Type type);
    Particle_Type get_payload_type() const;
    void set_damage(float value);
    float get_damage() const;
    void set_owner_type(Entity_Type owner);
    Entity_Type get_owner_type() const;
    void set_lifetime(float seconds);
    float get_lifetime() const;
    float get_age() const;
};

class Enemy : public Entity
{
private:
    // AI State Machine
    AI_State ai_state = AI_State::IDLE;
    AI_State previous_ai_state = AI_State::IDLE;

    // Target tracking
    glm::ivec2 target_position = {0, 0};
    glm::ivec2 home_position = {0, 0}; // Where enemy spawned/returns to
    glm::ivec2 patrol_target = {0, 0}; // Current patrol destination

    // Patrol system
    std::vector<glm::ivec2> patrol_points;
    int current_patrol_index = 0;
    bool patrol_forward = true; // Direction for ping-pong patrol

    // Range settings
    float attack_range = 40.0f;
    float detection_range = 200.0f;
    float flee_range = 300.0f;          // Max distance to chase before giving up
    float lose_interest_range = 350.0f; // Distance at which enemy loses interest

    // Combat settings
    float attack_damage = 10.0f;
    float attack_cooldown = 1.0f;
    float time_since_attack = 0.0f;

    // AI timing
    float state_timer = 0.0f;     // Time in current state
    float idle_duration = 2.0f;   // How long to stay idle
    float wander_duration = 3.0f; // How long to wander before stopping

    // Flee settings
    float flee_health_threshold = 0.25f; // Flee when health below 25%

    // State handlers
    void state_idle(float delta_time);
    void state_patrol(float delta_time);
    void state_chase(float delta_time);
    void state_attack(float delta_time);
    void state_flee(float delta_time);
    void state_return(float delta_time);

    // State transitions
    void transition_to(AI_State new_state);
    bool should_flee() const;
    bool can_attack() const;

    // Movement helpers
    void move_towards(const glm::ivec2 &target, float delta_time);
    void move_away_from(const glm::ivec2 &target, float delta_time);
    float distance_to(const glm::ivec2 &target) const;
    glm::ivec2 get_random_patrol_point() const;

public:
    Enemy();
    Enemy(std::string name, glm::vec2 coords);
    ~Enemy() = default;

    void update(float delta_time) override;

    // Sprite setup - for a standard 4-frame enemy sprite (128x32, 4 frames of 32x32)
    // Frames: 0=facing left, 1=facing right, 2=jumping, 3=hurt/dying
    void setup_enemy_sprite(const std::string &sprite_path);

    // Setters
    void set_target(const glm::ivec2 &target);
    void set_home_position(const glm::ivec2 &home);
    void add_patrol_point(const glm::ivec2 &point);
    void set_patrol_points(const std::vector<glm::ivec2> &points);
    void set_detection_range(float range);
    void set_attack_range(float range);
    void set_attack_damage(float damage);
    float get_attack_damage() const;

    // Getters
    AI_State get_ai_state() const;
    const char *get_ai_state_name() const;
    bool is_in_attack_range(const glm::ivec2 &target) const;
    bool is_in_detection_range(const glm::ivec2 &target) const;

    // Legacy compatibility
    void move_towards_target(float delta_time);
};

// NPC AI States for Devushki
enum class NPC_AI_State
{
    IDLE,    // Standing still
    FOLLOW,  // Following the player
    WANDER,  // Wandering around home position
    INTERACT // Interacting with the player
};

class Devushki : public Entity
{
private:
    // AI State Machine
    NPC_AI_State npc_ai_state = NPC_AI_State::IDLE;

    // Target tracking
    glm::ivec2 target_position = {0, 0};
    glm::ivec2 home_position = {0, 0};
    glm::ivec2 wander_target = {0, 0};

    // Range settings
    float follow_range = 200.0f;        // Start following player within this range
    float stop_follow_range = 60.0f;    // Stop when this close to player
    float lose_interest_range = 400.0f; // Stop following if player is too far
    float follow_speed_multiplier = 1.2f;

    // AI timing
    float state_timer = 0.0f;
    float idle_duration = 3.0f;
    float wander_duration = 4.0f;
    float interact_duration = 2.0f;

    // State handlers
    void state_idle(float delta_time);
    void state_follow(float delta_time);
    void state_wander(float delta_time);
    void state_interact(float delta_time);

    // State transitions
    void transition_to(NPC_AI_State new_state);

    // Movement helpers
    void move_towards(const glm::ivec2 &target, float delta_time);
    float distance_to(const glm::ivec2 &target) const;
    glm::ivec2 get_random_wander_point() const;

public:
    std::string name = "Devushki";

    Devushki();
    Devushki(std::string name, glm::vec2 coords);
    ~Devushki() = default;

    void update(float delta_time) override;

    // Setters
    void set_target(const glm::ivec2 &target);
    void set_home_position(const glm::ivec2 &home);
    void set_follow_range(float range);

    // Getters
    NPC_AI_State get_npc_ai_state() const;
    const char *get_npc_ai_state_name() const;
    bool is_in_follow_range(const glm::ivec2 &target) const;
};

// Boss AI States
enum class Boss_AI_State
{
    IDLE,   // Waiting for player
    CHASE,  // Pursuing the player
    ATTACK, // Melee attack
    SLAM,   // Ground slam AoE attack
    ENRAGE, // Enraged mode - faster and stronger
    DEAD    // Dead
};

class Boss : public Entity
{
private:
    // AI State Machine
    Boss_AI_State boss_ai_state = Boss_AI_State::IDLE;
    Boss_AI_State previous_boss_ai_state = Boss_AI_State::IDLE;

    // Target tracking
    glm::ivec2 target_position = {0, 0};
    glm::ivec2 home_position = {0, 0};

    // Range settings
    float attack_range = 60.0f;
    float detection_range = 350.0f;
    float slam_range = 120.0f; // AoE slam range
    float lose_interest_range = 500.0f;

    // Combat settings
    float attack_damage = 25.0f;
    float slam_damage = 40.0f;
    float attack_cooldown = 1.5f;
    float slam_cooldown = 5.0f;
    float time_since_attack = 0.0f;
    float time_since_slam = 0.0f;

    // AI timing
    float state_timer = 0.0f;

    // Enrage settings
    float enrage_health_threshold = 0.3f; // Enrage below 30% health
    bool is_enraged = false;
    float enrage_speed_multiplier = 1.5f;
    float enrage_damage_multiplier = 1.5f;

    // State handlers
    void state_idle(float delta_time);
    void state_chase(float delta_time);
    void state_attack(float delta_time);
    void state_slam(float delta_time);
    void state_enrage(float delta_time);

    // State transitions
    void transition_to(Boss_AI_State new_state);
    bool should_enrage() const;
    bool can_attack() const;
    bool can_slam() const;

    // Movement helpers
    void move_towards(const glm::ivec2 &target, float delta_time);
    float distance_to(const glm::ivec2 &target) const;

public:
    std::string name = "Boss";

    Boss();
    Boss(std::string name, glm::vec2 coords);
    ~Boss() = default;

    void update(float delta_time) override;

    // Setters
    void set_target(const glm::ivec2 &target);
    void set_home_position(const glm::ivec2 &home);
    void set_detection_range(float range);
    void set_attack_range(float range);
    void set_attack_damage(float damage);
    void set_slam_damage(float damage);
    float get_attack_damage() const;

    // Getters
    Boss_AI_State get_boss_ai_state() const;
    const char *get_boss_ai_state_name() const;
    bool is_in_attack_range(const glm::ivec2 &target) const;
    bool is_in_detection_range(const glm::ivec2 &target) const;
    bool get_is_enraged() const;
};
