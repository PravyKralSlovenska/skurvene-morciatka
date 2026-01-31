#include "engine/player/entity.hpp"

#include <cmath>
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "others/GLOBALS.hpp"

// Static ID counter
int Entity::next_id = 1;

// ==================== Entity ====================

Entity::Entity()
    : ID(next_id++)
{
    calculate_hitbox();
}

void Entity::update(float delta_time)
{
    update_physics(delta_time);
    calculate_hitbox();

    // Update state based on velocity and ground
    if (velocity.y > 10.0f)
    {
        state = Entity_States::FALLING;
    }
    else if (velocity.y < -10.0f)
    {
        state = Entity_States::JUMPING;
    }
    else if (std::abs(velocity.x) > 0.1f)
    {
        state = Entity_States::WALKING;
    }
    else
    {
        state = Entity_States::STILL;
    }
}

void Entity::update_physics(float delta_time)
{
    // Without world reference, just apply velocity directly (no collision)
    if (!world_ref)
    {
        coords.x += static_cast<int>(velocity.x * delta_time);
        coords.y += static_cast<int>(velocity.y * delta_time);
        velocity.x *= 0.9f;
        velocity.y *= 0.9f;
        calculate_hitbox();
        return;
    }

    // Apply gravity if enabled and not in noclip
    if (gravity_enabled && !noclip)
    {
        apply_gravity(GRAVITY, delta_time);
    }

    // Calculate new position
    glm::ivec2 new_pos = coords;
    new_pos.x += static_cast<int>(velocity.x * delta_time);
    new_pos.y += static_cast<int>(velocity.y * delta_time);

    // Handle collision
    if (!noclip)
    {
        resolve_collision(new_pos);
    }

    coords = new_pos;

    // Apply friction/drag
    velocity.x *= 0.9f;
    if (on_ground)
    {
        velocity.y = 0.0f;
    }
}

void Entity::set_hitbox_dimensions(const int width, const int height)
{
    set_hitbox_dimensions(glm::ivec2(width, height));
}

void Entity::set_hitbox_dimensions(const glm::ivec2 &hitbox_dimensions)
{
    this->hitbox_dimensions = hitbox_dimensions;
    hitbox_dimensions_half = this->hitbox_dimensions / 2;
    calculate_hitbox();
}

void Entity::set_position(const int x, const int y)
{
    set_position(glm::ivec2(x, y));
}

void Entity::set_position(const glm::ivec2 &position)
{
    this->coords = position;
    calculate_hitbox();
}

void Entity::set_sprite_file(const std::string &path)
{
    entity_sprite = path;
}

void Entity::set_velocity(float vx, float vy)
{
    velocity = {vx, vy};
}

void Entity::set_velocity(const glm::vec2 &vel)
{
    velocity = vel;
}

void Entity::calculate_hitbox()
{
    // lavy horny roh
    hitbox[0] = glm::ivec2(coords.x - hitbox_dimensions_half.x, coords.y - hitbox_dimensions_half.y);
    // pravy horny roh
    hitbox[1] = glm::ivec2(coords.x + hitbox_dimensions_half.x, coords.y - hitbox_dimensions_half.y);
    // lavy dolny roh
    hitbox[2] = glm::ivec2(coords.x - hitbox_dimensions_half.x, coords.y + hitbox_dimensions_half.y);
    // pravy dolny roh
    hitbox[3] = glm::ivec2(coords.x + hitbox_dimensions_half.x, coords.y + hitbox_dimensions_half.y);
}

int Entity::get_id() const
{
    return ID;
}

bool Entity::get_is_alive() const
{
    return is_alive;
}

Entity_States Entity::get_state() const
{
    return state;
}

bool Entity::get_noclip() const
{
    return noclip;
}

glm::ivec2 Entity::get_chunk_position(int chunk_pixel_width, int chunk_pixel_height) const
{
    return glm::ivec2{
        static_cast<int>(std::floor(static_cast<float>(coords.x) / chunk_pixel_width)),
        static_cast<int>(std::floor(static_cast<float>(coords.y) / chunk_pixel_height))};
}

void Entity::take_damage(float damage)
{
    healthpoints -= damage;
    state = Entity_States::HIT;

    if (healthpoints <= 0)
    {
        die();
    }
}

void Entity::heal(float amount)
{
    healthpoints = std::min(healthpoints + amount, max_healthpoints);
}

void Entity::die()
{
    is_alive = false;
    state = Entity_States::DEAD;
    velocity = {0.0f, 0.0f};
}

void Entity::move_left()
{
    // Set velocity - physics will apply it
    velocity.x = -speed;
}

void Entity::move_right()
{
    // Set velocity - physics will apply it
    velocity.x = speed;
}

void Entity::move_up()
{
    // Without world, allow direct up movement
    if (!world_ref)
    {
        velocity.y = -speed;
        return;
    }

    // Jump if on ground, or fly up in noclip mode
    if (on_ground || noclip)
    {
        velocity.y = -speed * 2.5f;
        on_ground = false;
    }
}

void Entity::move_down()
{
    // Without world, allow direct down movement
    if (!world_ref)
    {
        velocity.y = speed;
        return;
    }

    // Only allow downward movement in noclip mode (gravity handles falling)
    if (noclip)
    {
        velocity.y = speed;
    }
}

void Entity::jump()
{
    if (on_ground || noclip)
    {
        velocity.y = -speed * 2.5f;
        state = Entity_States::JUMPING;
        on_ground = false;
    }
}

void Entity::go_to(const glm::ivec2 &target)
{
    coords = target;
    calculate_hitbox();
}

void Entity::move_to(const glm::ivec2 &target)
{
    glm::vec2 direction = glm::vec2(target - coords);
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (length > 0.0f)
    {
        direction /= length;
        velocity = direction * speed;
    }
}

void Entity::move_by(float dx, float dy)
{
    coords.x += static_cast<int>(dx);
    coords.y += static_cast<int>(dy);
    calculate_hitbox();
}

void Entity::apply_gravity(float gravity, float delta_time)
{
    velocity.y += gravity * delta_time;
    // Terminal velocity
    if (velocity.y > 800.0f)
        velocity.y = 800.0f;
}

void Entity::apply_velocity(float delta_time)
{
    coords.x += static_cast<int>(velocity.x * delta_time);
    coords.y += static_cast<int>(velocity.y * delta_time);

    // Apply friction/drag
    velocity *= 0.98f;
}

void Entity::set_world(World *world)
{
    world_ref = world;
}

void Entity::set_noclip(bool enabled)
{
    noclip = enabled;
}

void Entity::toggle_noclip()
{
    noclip = !noclip;
}

// ==================== Collision Detection ====================

bool Entity::is_solid_at(int world_x, int world_y) const
{
    if (!world_ref)
        return false;

    int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
    glm::ivec2 chunk_dims = world_ref->get_chunk_dimensions();
    int chunk_pixel_width = chunk_dims.x * particle_size;
    int chunk_pixel_height = chunk_dims.y * particle_size;

    // Get chunk coordinates
    glm::ivec2 chunk_pos{
        static_cast<int>(std::floor(static_cast<float>(world_x) / chunk_pixel_width)),
        static_cast<int>(std::floor(static_cast<float>(world_y) / chunk_pixel_height))};

    Chunk *chunk = world_ref->get_chunk(chunk_pos);
    if (!chunk)
        return false;

    // Get local cell position within chunk
    int local_x = (world_x - chunk_pos.x * chunk_pixel_width) / particle_size;
    int local_y = (world_y - chunk_pos.y * chunk_pixel_height) / particle_size;

    // Bounds check
    if (local_x < 0 || local_x >= chunk_dims.x || local_y < 0 || local_y >= chunk_dims.y)
        return false;

    WorldCell *cell = chunk->get_worldcell(local_x, local_y);
    if (!cell)
        return false;

    // Check if particle is solid
    return cell->particle.state == Particle_State::SOLID;
}

bool Entity::check_collision_at(const glm::ivec2 &position) const
{
    if (!world_ref || noclip)
        return false;

    // Check corners and edges of hitbox
    int left = position.x - hitbox_dimensions_half.x;
    int right = position.x + hitbox_dimensions_half.x;
    int top = position.y - hitbox_dimensions_half.y;
    int bottom = position.y + hitbox_dimensions_half.y;

    int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);

    // Check bottom edge (feet)
    for (int x = left; x <= right; x += particle_size)
    {
        if (is_solid_at(x, bottom))
            return true;
    }
    if (is_solid_at(right, bottom))
        return true;

    // Check top edge (head)
    for (int x = left; x <= right; x += particle_size)
    {
        if (is_solid_at(x, top))
            return true;
    }

    // Check left edge
    for (int y = top; y <= bottom; y += particle_size)
    {
        if (is_solid_at(left, y))
            return true;
    }

    // Check right edge
    for (int y = top; y <= bottom; y += particle_size)
    {
        if (is_solid_at(right, y))
            return true;
    }

    return false;
}

int Entity::get_ground_height_at(int world_x, int start_y, int max_check) const
{
    if (!world_ref)
        return start_y;

    int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);

    // Search downward for ground
    for (int y = start_y; y < start_y + max_check * particle_size; y += particle_size)
    {
        if (is_solid_at(world_x, y))
        {
            return y;
        }
    }

    return start_y + max_check * particle_size; // No ground found
}

bool Entity::can_move_to(const glm::ivec2 &new_pos) const
{
    if (noclip)
        return true;
    if (!world_ref)
        return true;

    // Check if there's collision at new position
    if (check_collision_at(new_pos))
        return false;

    return true;
}

void Entity::resolve_collision(glm::ivec2 &new_pos)
{
    if (!world_ref || noclip)
        return;

    glm::ivec2 old_pos = coords;
    int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);

    on_ground = false;

    // Resolve horizontal movement first
    glm::ivec2 test_pos = {new_pos.x, old_pos.y};

    if (check_collision_at(test_pos))
    {
        // Check if we can step up (climb small obstacles)
        bool can_step = false;
        for (int step = particle_size; step <= MAX_STEP_HEIGHT * particle_size; step += particle_size)
        {
            glm::ivec2 step_pos = {new_pos.x, old_pos.y - step};
            if (!check_collision_at(step_pos))
            {
                // Can step up
                new_pos.y = old_pos.y - step;
                can_step = true;
                break;
            }
        }

        if (!can_step)
        {
            // Can't move horizontally, block X movement
            new_pos.x = old_pos.x;
            velocity.x = 0.0f;
        }
    }

    // Resolve vertical movement
    test_pos = {new_pos.x, new_pos.y};

    if (check_collision_at(test_pos))
    {
        if (velocity.y > 0) // Moving down (falling)
        {
            // Find the ground
            int bottom = old_pos.y + hitbox_dimensions_half.y;

            // Binary search for exact ground position
            int search_y = old_pos.y;
            while (search_y < new_pos.y)
            {
                glm::ivec2 search_pos = {new_pos.x, search_y + 1};
                if (check_collision_at(search_pos))
                {
                    new_pos.y = search_y;
                    break;
                }
                search_y++;
            }

            velocity.y = 0.0f;
            on_ground = true;
        }
        else if (velocity.y < 0) // Moving up (jumping)
        {
            // Hit ceiling
            new_pos.y = old_pos.y;
            velocity.y = 0.0f;
        }
    }

    // Final ground check
    glm::ivec2 feet_check = {new_pos.x, new_pos.y + hitbox_dimensions_half.y + 1};
    if (is_solid_at(feet_check.x, feet_check.y) ||
        is_solid_at(new_pos.x - hitbox_dimensions_half.x, feet_check.y) ||
        is_solid_at(new_pos.x + hitbox_dimensions_half.x, feet_check.y))
    {
        on_ground = true;
    }
}

// ==================== Player ====================

Player::Player(std::string name, glm::vec2 coords)
    : name(name)
{
    this->coords = coords;
    this->type = Entity_Type::PLAYER;
    this->max_healthpoints = 100.0f;
    this->healthpoints = 100.0f;
    this->speed = 200.0f; // Movement speed in pixels per second
    set_hitbox_dimensions(32, 48);
}

void Player::update(float delta_time)
{
    // Player physics - gravity and collision
    update_physics(delta_time);

    // Update state based on movement
    calculate_hitbox();

    // Update animation state
    if (!on_ground && velocity.y < 0)
    {
        state = Entity_States::JUMPING;
    }
    else if (!on_ground && velocity.y > 0)
    {
        state = Entity_States::FALLING;
    }
    else if (std::abs(velocity.x) > 1.0f)
    {
        state = Entity_States::WALKING;
    }
    else
    {
        state = Entity_States::STILL;
    }
}

void Player::change_selected_item(const int inventory_slot)
{
    if (inventory_slot >= 0 && inventory_slot < 50)
    {
        selected_item = inventory_slot;
    }
}

// ==================== Enemy ====================

Enemy::Enemy()
{
    type = Entity_Type::ENEMY;
    max_healthpoints = 50.0f;
    healthpoints = 50.0f;
    speed = 50.0f;
    set_hitbox_dimensions(24, 24);
    ai_state = AI_State::IDLE;
}

Enemy::Enemy(std::string name, glm::vec2 coords)
    : Enemy()
{
    this->coords = coords;
    this->home_position = coords;
}

void Enemy::update(float delta_time)
{
    if (!is_alive)
    {
        ai_state = AI_State::DEAD;
        return;
    }

    // Update timers
    time_since_attack += delta_time;
    state_timer += delta_time;

    // Check for flee condition (can interrupt any state except DEAD)
    if (should_flee() && ai_state != AI_State::FLEE)
    {
        transition_to(AI_State::FLEE);
    }

    // Run current state behavior
    switch (ai_state)
    {
    case AI_State::IDLE:
        state_idle(delta_time);
        break;
    case AI_State::PATROL:
        state_patrol(delta_time);
        break;
    case AI_State::CHASE:
        state_chase(delta_time);
        break;
    case AI_State::ATTACK:
        state_attack(delta_time);
        break;
    case AI_State::FLEE:
        state_flee(delta_time);
        break;
    case AI_State::RETURN:
        state_return(delta_time);
        break;
    case AI_State::DEAD:
        velocity = {0.0f, 0.0f};
        break;
    }

    // Apply physics (gravity and collision)
    update_physics(delta_time);
    calculate_hitbox();
}

// ==================== State Handlers ====================

void Enemy::state_idle(float delta_time)
{
    velocity *= 0.9f; // Slow down
    state = Entity_States::STILL;

    // Check if player is in detection range
    if (is_in_detection_range(target_position))
    {
        transition_to(AI_State::CHASE);
        return;
    }

    // After idle duration, start patrolling
    if (state_timer >= idle_duration)
    {
        if (!patrol_points.empty())
        {
            transition_to(AI_State::PATROL);
        }
        else
        {
            // No patrol points - generate random wander target
            patrol_target = get_random_patrol_point();
            transition_to(AI_State::PATROL);
        }
    }
}

void Enemy::state_patrol(float delta_time)
{
    state = Entity_States::WALKING;

    // Check if player is in detection range - interrupt patrol
    if (is_in_detection_range(target_position))
    {
        transition_to(AI_State::CHASE);
        return;
    }

    // Move towards patrol target
    float dist = distance_to(patrol_target);

    if (dist < 10.0f) // Reached patrol point
    {
        if (!patrol_points.empty())
        {
            // Move to next patrol point (ping-pong pattern)
            if (patrol_forward)
            {
                current_patrol_index++;
                if (current_patrol_index >= static_cast<int>(patrol_points.size()))
                {
                    current_patrol_index = static_cast<int>(patrol_points.size()) - 2;
                    patrol_forward = false;
                    if (current_patrol_index < 0)
                        current_patrol_index = 0;
                }
            }
            else
            {
                current_patrol_index--;
                if (current_patrol_index < 0)
                {
                    current_patrol_index = 1;
                    patrol_forward = true;
                    if (current_patrol_index >= static_cast<int>(patrol_points.size()))
                        current_patrol_index = 0;
                }
            }
            patrol_target = patrol_points[current_patrol_index];
        }
        else
        {
            // Random wandering - go back to idle
            transition_to(AI_State::IDLE);
            return;
        }
    }

    move_towards(patrol_target, delta_time);

    // Timeout - go back to idle
    if (state_timer >= wander_duration && patrol_points.empty())
    {
        transition_to(AI_State::IDLE);
    }
}

void Enemy::state_chase(float delta_time)
{
    state = Entity_States::WALKING;

    float dist = distance_to(target_position);

    // Lost interest - target too far
    if (dist > lose_interest_range)
    {
        transition_to(AI_State::RETURN);
        return;
    }

    // In attack range - switch to attack
    if (dist <= attack_range)
    {
        transition_to(AI_State::ATTACK);
        return;
    }

    // Chase the target
    move_towards(target_position, delta_time);
}

void Enemy::state_attack(float delta_time)
{
    state = Entity_States::STILL;
    velocity *= 0.5f; // Slow down during attack

    float dist = distance_to(target_position);

    // Target moved out of attack range
    if (dist > attack_range * 1.2f) // Small buffer to prevent flickering
    {
        transition_to(AI_State::CHASE);
        return;
    }

    // Target moved out of detection range entirely
    if (dist > detection_range)
    {
        transition_to(AI_State::RETURN);
        return;
    }

    // Perform attack if cooldown ready
    if (can_attack())
    {
        // Attack! (damage will be handled by entity_manager or collision system)
        time_since_attack = 0.0f;
        state = Entity_States::HIT; // Visual feedback

        // TODO: Actually deal damage to player through entity_manager
        // For now, just log
        // std::cout << "Enemy " << ID << " attacks for " << attack_damage << " damage!\n";
    }
}

void Enemy::state_flee(float delta_time)
{
    state = Entity_States::WALKING;

    // Run away from target
    move_away_from(target_position, delta_time);

    // If far enough from target, try to return home
    float dist = distance_to(target_position);
    if (dist > flee_range)
    {
        transition_to(AI_State::RETURN);
    }

    // If health recovered (e.g., from healing), stop fleeing
    if (!should_flee())
    {
        transition_to(AI_State::IDLE);
    }
}

void Enemy::state_return(float delta_time)
{
    state = Entity_States::WALKING;

    // If player comes into range while returning, chase them
    if (is_in_detection_range(target_position) && !should_flee())
    {
        transition_to(AI_State::CHASE);
        return;
    }

    // Move towards home position
    float dist = distance_to(home_position);

    if (dist < 15.0f)
    {
        // Reached home
        transition_to(AI_State::IDLE);
        return;
    }

    move_towards(home_position, delta_time);
}

// ==================== State Transitions ====================

void Enemy::transition_to(AI_State new_state)
{
    if (ai_state == new_state)
        return;

    previous_ai_state = ai_state;
    ai_state = new_state;
    state_timer = 0.0f;

    // State entry actions
    switch (new_state)
    {
    case AI_State::IDLE:
        velocity = {0.0f, 0.0f};
        break;
    case AI_State::PATROL:
        if (patrol_points.empty())
        {
            patrol_target = get_random_patrol_point();
        }
        else
        {
            patrol_target = patrol_points[current_patrol_index];
        }
        break;
    case AI_State::CHASE:
        // Increase speed slightly when chasing
        break;
    case AI_State::ATTACK:
        velocity = {0.0f, 0.0f};
        break;
    case AI_State::FLEE:
        // Could increase speed when fleeing
        break;
    case AI_State::RETURN:
        break;
    case AI_State::DEAD:
        velocity = {0.0f, 0.0f};
        break;
    }
}

bool Enemy::should_flee() const
{
    return (healthpoints / max_healthpoints) <= flee_health_threshold;
}

bool Enemy::can_attack() const
{
    return time_since_attack >= attack_cooldown;
}

// ==================== Movement Helpers ====================

void Enemy::move_towards(const glm::ivec2 &target, float delta_time)
{
    glm::vec2 direction = glm::vec2(target - coords);
    float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (dist > 1.0f)
    {
        direction /= dist;
        velocity = direction * speed;
    }
}

void Enemy::move_away_from(const glm::ivec2 &target, float delta_time)
{
    glm::vec2 direction = glm::vec2(coords - target); // Reversed direction
    float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (dist > 0.1f)
    {
        direction /= dist;
        velocity = direction * speed * 1.2f; // Run faster when fleeing
    }
}

float Enemy::distance_to(const glm::ivec2 &target) const
{
    glm::vec2 diff = glm::vec2(target - coords);
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

glm::ivec2 Enemy::get_random_patrol_point() const
{
    // Generate random point within reasonable range of home
    float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f; // 2*PI
    float distance = 50.0f + static_cast<float>(rand()) / RAND_MAX * 100.0f;

    return glm::ivec2{
        home_position.x + static_cast<int>(std::cos(angle) * distance),
        home_position.y + static_cast<int>(std::sin(angle) * distance)};
}

// ==================== Public Methods ====================

void Enemy::set_target(const glm::ivec2 &target)
{
    target_position = target;
}

void Enemy::set_home_position(const glm::ivec2 &home)
{
    home_position = home;
}

void Enemy::add_patrol_point(const glm::ivec2 &point)
{
    patrol_points.push_back(point);
}

void Enemy::set_patrol_points(const std::vector<glm::ivec2> &points)
{
    patrol_points = points;
    current_patrol_index = 0;
    patrol_forward = true;
}

void Enemy::set_detection_range(float range)
{
    detection_range = range;
}

void Enemy::set_attack_range(float range)
{
    attack_range = range;
}

void Enemy::set_attack_damage(float damage)
{
    attack_damage = damage;
}

AI_State Enemy::get_ai_state() const
{
    return ai_state;
}

const char *Enemy::get_ai_state_name() const
{
    switch (ai_state)
    {
    case AI_State::IDLE:
        return "IDLE";
    case AI_State::PATROL:
        return "PATROL";
    case AI_State::CHASE:
        return "CHASE";
    case AI_State::ATTACK:
        return "ATTACK";
    case AI_State::FLEE:
        return "FLEE";
    case AI_State::RETURN:
        return "RETURN";
    case AI_State::DEAD:
        return "DEAD";
    default:
        return "UNKNOWN";
    }
}

bool Enemy::is_in_attack_range(const glm::ivec2 &target) const
{
    return distance_to(target) <= attack_range;
}

bool Enemy::is_in_detection_range(const glm::ivec2 &target) const
{
    return distance_to(target) <= detection_range;
}

void Enemy::move_towards_target(float delta_time)
{
    move_towards(target_position, delta_time);
}
