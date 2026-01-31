#pragma once

#include <iostream>
#include <array>
#include <functional>
#include <vector>
#include <glm/glm.hpp>

#include "engine/player/inventory.hpp"

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
    PROJECTILE
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

    // health/damage
    void take_damage(float damage);
    void heal(float amount);
    void die();

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

public:
    std::string name;
    int selected_item = 0;

public:
    Player(std::string name, glm::vec2 coords);
    ~Player() = default;

    void update(float delta_time) override;

    void change_selected_item(const int inventory_slot);
    void change_selected_item(const Item item);
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

    // Setters
    void set_target(const glm::ivec2 &target);
    void set_home_position(const glm::ivec2 &home);
    void add_patrol_point(const glm::ivec2 &point);
    void set_patrol_points(const std::vector<glm::ivec2> &points);
    void set_detection_range(float range);
    void set_attack_range(float range);
    void set_attack_damage(float damage);

    // Getters
    AI_State get_ai_state() const;
    const char *get_ai_state_name() const;
    bool is_in_attack_range(const glm::ivec2 &target) const;
    bool is_in_detection_range(const glm::ivec2 &target) const;

    // Legacy compatibility
    void move_towards_target(float delta_time);
};
