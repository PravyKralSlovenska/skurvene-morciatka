#include "engine/player/entity.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "others/GLOBALS.hpp"

// Static ID counter
int Entity::next_id = 1;

namespace
{
    glm::vec2 safe_normalize_vec2(const glm::vec2 &v, const glm::vec2 &fallback)
    {
        const float len = std::sqrt(v.x * v.x + v.y * v.y);
        if (len <= 0.001f)
            return fallback;
        return v / len;
    }

    glm::vec2 rotate_vec2(const glm::vec2 &v, float angle)
    {
        const float c = std::cos(angle);
        const float s = std::sin(angle);
        return glm::vec2(v.x * c - v.y * s, v.x * s + v.y * c);
    }

    bool world_pos_to_chunk_local(World *world, const glm::ivec2 &world_pos,
                                  glm::ivec2 &out_chunk_pos, glm::ivec2 &out_local_pos)
    {
        if (!world)
            return false;

        const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        const int chunk_pixel_w = chunk_dims.x * particle_size;
        const int chunk_pixel_h = chunk_dims.y * particle_size;

        out_chunk_pos = glm::ivec2(
            static_cast<int>(std::floor(static_cast<float>(world_pos.x) / chunk_pixel_w)),
            static_cast<int>(std::floor(static_cast<float>(world_pos.y) / chunk_pixel_h)));

        int pixel_offset_x = world_pos.x - out_chunk_pos.x * chunk_pixel_w;
        int pixel_offset_y = world_pos.y - out_chunk_pos.y * chunk_pixel_h;
        out_local_pos = glm::ivec2(pixel_offset_x / particle_size, pixel_offset_y / particle_size);

        if (out_local_pos.x < 0)
            out_local_pos.x += chunk_dims.x;
        if (out_local_pos.y < 0)
            out_local_pos.y += chunk_dims.y;

        return out_local_pos.x >= 0 && out_local_pos.x < chunk_dims.x &&
               out_local_pos.y >= 0 && out_local_pos.y < chunk_dims.y;
    }

    void heat_and_ignite_particles(World *world, const glm::ivec2 &center_pos, int radius_cells,
                                   float added_temperature_c, float ignite_temperature_c)
    {
        if (!world || radius_cells < 0)
            return;

        const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);

        for (int dy = -radius_cells; dy <= radius_cells; ++dy)
        {
            for (int dx = -radius_cells; dx <= radius_cells; ++dx)
            {
                if (dx * dx + dy * dy > radius_cells * radius_cells)
                    continue;

                const glm::ivec2 sample_pos = center_pos + glm::ivec2(dx * particle_size, dy * particle_size);

                glm::ivec2 chunk_pos;
                glm::ivec2 local_pos;
                if (!world_pos_to_chunk_local(world, sample_pos, chunk_pos, local_pos))
                    continue;

                Chunk *chunk = world->get_chunk(chunk_pos);
                if (!chunk)
                    continue;

                WorldCell *cell = chunk->get_worldcell(local_pos.x, local_pos.y);
                if (!cell)
                    continue;

                if (cell->particle.type == Particle_Type::EMPTY)
                    continue;

                cell->particle.physics.temperature = std::clamp(
                    cell->particle.physics.temperature + added_temperature_c,
                    -273.15f,
                    cell->particle.physics.max_temperature);

                chunk->mark_dirty();

                if (cell->particle.flags.is_flammable &&
                    cell->particle.physics.temperature >= ignite_temperature_c)
                {
                    chunk->set_worldcell(local_pos, Particle_Type::FIRE, false);
                }
            }
        }
    }
} // namespace

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
    // In noclip mode, movement should come only from current input.
    // No gravity, collision response, friction, or momentum carryover.
    if (noclip)
    {
        coords.x += static_cast<int>(velocity.x * delta_time);
        coords.y += static_cast<int>(velocity.y * delta_time);
        velocity = {0.0f, 0.0f};
        acceleration = {0.0f, 0.0f};
        on_ground = false;
        calculate_hitbox();
        return;
    }

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

void Entity::setup_sprite_sheet(const std::string &path, int sheet_width, int sheet_height,
                                int frame_width, int frame_height, int num_frames)
{
    entity_sprite = path;
    sprite_animation.setup_sheet(path, sheet_width, sheet_height, frame_width, frame_height, num_frames);
    use_sprite_animation = true;
}

void Entity::update_sprite_state()
{
    if (!use_sprite_animation)
        return;

    // Determine sprite state based on entity state and velocity
    Sprite_State new_sprite_state;

    // If hurt/dead, use hurt sprite
    if (state == Entity_States::HIT || state == Entity_States::DEAD || !is_alive || damage_flash_timer > 0.0f)
    {
        new_sprite_state = Sprite_State::HURT;
    }
    // If jumping or falling, use jump sprite
    else if (state == Entity_States::JUMPING || state == Entity_States::FALLING || !on_ground)
    {
        new_sprite_state = Sprite_State::JUMPING;
    }
    // Otherwise, use left/right based on velocity
    else if (velocity.x < 0)
    {
        new_sprite_state = Sprite_State::FACING_LEFT;
    }
    else
    {
        new_sprite_state = Sprite_State::FACING_RIGHT;
    }

    sprite_animation.set_state(new_sprite_state);
}

Sprite_Animation &Entity::get_sprite_animation()
{
    return sprite_animation;
}

bool Entity::has_sprite_animation() const
{
    return use_sprite_animation;
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
    if (damage <= 0.0f || !can_take_damage())
        return;

    healthpoints -= damage;
    damage_invuln_timer = damage_invuln_duration;
    damage_flash_timer = damage_flash_duration;
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

void Entity::update_damage_timers(float delta_time)
{
    if (damage_invuln_timer > 0.0f)
    {
        damage_invuln_timer -= delta_time;
        if (damage_invuln_timer < 0.0f)
            damage_invuln_timer = 0.0f;
    }

    if (damage_flash_timer > 0.0f)
    {
        damage_flash_timer -= delta_time;
        if (damage_flash_timer < 0.0f)
            damage_flash_timer = 0.0f;
    }
}

bool Entity::can_take_damage() const
{
    return is_alive && damage_invuln_timer <= 0.0f;
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

    if (noclip)
    {
        // Entering fly mode should immediately discard existing forces.
        velocity = {0.0f, 0.0f};
        acceleration = {0.0f, 0.0f};
        on_ground = false;
    }
}

void Entity::toggle_noclip()
{
    set_noclip(!noclip);
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

bool Entity::is_valid_spawn_position(const glm::ivec2 &position) const
{
    if (!world_ref)
        return true; // No world = no collision to check

    // Check if the entire hitbox area is free of solid particles
    int left = position.x - hitbox_dimensions_half.x;
    int right = position.x + hitbox_dimensions_half.x;
    int top = position.y - hitbox_dimensions_half.y;
    int bottom = position.y + hitbox_dimensions_half.y;

    int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);

    // Check the full hitbox interior, not just edges
    for (int y = top; y <= bottom; y += particle_size)
    {
        for (int x = left; x <= right; x += particle_size)
        {
            if (is_solid_at(x, y))
                return false;
        }
        // Check right edge explicitly
        if (is_solid_at(right, y))
            return false;
    }
    // Check bottom edge explicitly
    for (int x = left; x <= right; x += particle_size)
    {
        if (is_solid_at(x, bottom))
            return false;
    }
    if (is_solid_at(right, bottom))
        return false;

    return true;
}

glm::ivec2 Entity::find_valid_spawn_position(const glm::ivec2 &desired_pos, int max_search_radius) const
{
    if (!world_ref)
        return desired_pos;

    // First check the desired position
    if (is_valid_spawn_position(desired_pos))
        return desired_pos;

    int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
    int step = particle_size * 2;

    // Spiral search outward from desired position
    for (int radius = step; radius <= max_search_radius; radius += step)
    {
        // Check positions in a square ring at this radius
        for (int dx = -radius; dx <= radius; dx += step)
        {
            for (int dy = -radius; dy <= radius; dy += step)
            {
                // Only check the outer ring
                if (std::abs(dx) != radius && std::abs(dy) != radius)
                    continue;

                glm::ivec2 test_pos = {desired_pos.x + dx, desired_pos.y + dy};
                if (is_valid_spawn_position(test_pos))
                    return test_pos;
            }
        }
    }

    // Fallback: return desired position (will clip through terrain)
    return desired_pos;
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

// ==================== Projectile ====================

Projectile::Projectile()
{
    type = Entity_Type::PROJECTILE;
    speed = 700.0f;
    max_healthpoints = 1.0f;
    healthpoints = 1.0f;
    set_hitbox_dimensions(6, 6);
}

Projectile::Projectile(const glm::vec2 &position, const glm::vec2 &velocity, Particle_Type payload_type)
    : Projectile()
{
    coords = glm::ivec2(position);
    this->velocity = velocity;
    this->payload_type = payload_type;
    calculate_hitbox();
}

void Projectile::update(float delta_time)
{
    if (!is_alive)
        return;

    age_seconds += delta_time;
    if (age_seconds >= lifetime_seconds)
    {
        die();
        return;
    }

    apply_gravity(GRAVITY * gravity_multiplier, delta_time);

    glm::vec2 displacement = velocity * delta_time;
    int particle_size = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
    int steps = std::max(1, static_cast<int>(std::ceil(
                                std::max(std::abs(displacement.x), std::abs(displacement.y)) /
                                static_cast<float>(particle_size))));

    glm::vec2 step_delta = displacement / static_cast<float>(steps);
    glm::vec2 simulated_pos = glm::vec2(coords);

    for (int i = 0; i < steps; ++i)
    {
        simulated_pos += step_delta;
        glm::ivec2 next_pos = glm::ivec2(simulated_pos);

        if (check_collision_at(next_pos))
        {
            if (world_ref && world_impact_enabled)
            {
                if (payload_type == Particle_Type::FIRE)
                {
                    // Flamethrower only ignites flammable materials (e.g., wood).
                    heat_and_ignite_particles(world_ref, next_pos, 1, 75.0f, 300.0f);
                }
                else
                {
                    static constexpr int IMPACT_DELETE_RADIUS_CELLS = 2;
                    for (int dy = -IMPACT_DELETE_RADIUS_CELLS; dy <= IMPACT_DELETE_RADIUS_CELLS; ++dy)
                    {
                        for (int dx = -IMPACT_DELETE_RADIUS_CELLS; dx <= IMPACT_DELETE_RADIUS_CELLS; ++dx)
                        {
                            if (dx * dx + dy * dy > IMPACT_DELETE_RADIUS_CELLS * IMPACT_DELETE_RADIUS_CELLS)
                                continue;

                            glm::ivec2 delete_pos = next_pos + glm::ivec2(dx * particle_size, dy * particle_size);
                            world_ref->place_particle(delete_pos, Particle_Type::EMPTY);
                        }
                    }
                }
            }
            die();
            calculate_hitbox();
            return;
        }

        coords = next_pos;

        if (world_ref && payload_type == Particle_Type::FIRE)
        {
            // Do not paint fire into the world; only ignite flammable particles.
            heat_and_ignite_particles(world_ref, coords, 0, 22.0f, 300.0f);
        }
    }

    velocity *= air_drag;
    state = Entity_States::WALKING;
    calculate_hitbox();
}

void Projectile::set_payload_type(Particle_Type type)
{
    payload_type = type;
}

Particle_Type Projectile::get_payload_type() const
{
    return payload_type;
}

void Projectile::set_damage(float value)
{
    damage = std::max(0.0f, value);
}

float Projectile::get_damage() const
{
    return damage;
}

void Projectile::set_owner_type(Entity_Type owner)
{
    owner_type = owner;
}

Entity_Type Projectile::get_owner_type() const
{
    return owner_type;
}

void Projectile::set_lifetime(float seconds)
{
    lifetime_seconds = std::max(0.05f, seconds);
}

float Projectile::get_lifetime() const
{
    return lifetime_seconds;
}

float Projectile::get_age() const
{
    return age_seconds;
}

void Projectile::set_gravity_multiplier(float value)
{
    gravity_multiplier = std::clamp(value, -1.5f, 2.0f);
}

float Projectile::get_gravity_multiplier() const
{
    return gravity_multiplier;
}

void Projectile::set_air_drag(float value)
{
    air_drag = std::clamp(value, 0.7f, 1.0f);
}

float Projectile::get_air_drag() const
{
    return air_drag;
}

void Projectile::set_world_impact_enabled(bool enabled)
{
    world_impact_enabled = enabled;
}

bool Projectile::is_world_impact_enabled() const
{
    return world_impact_enabled;
}

// ==================== Enemy ====================

Enemy::Enemy()
{
    type = Entity_Type::ENEMY;
    max_healthpoints = 50.0f;
    healthpoints = 50.0f;
    speed = 50.0f;
    set_hitbox_dimensions(40, 40);
    ai_state = AI_State::IDLE;
}

Enemy::Enemy(std::string name, glm::vec2 coords)
    : Enemy()
{
    this->coords = coords;
    this->home_position = coords;
}

void Enemy::setup_enemy_sprite(const std::string &sprite_path)
{
    // Standard enemy sprite sheet: 128x32 pixels, 4 frames of 32x32
    // Frame 0: facing left
    // Frame 1: facing right
    // Frame 2: jumping/falling
    // Frame 3: hurt/dying
    setup_sprite_sheet(sprite_path, 128, 32, 32, 32, 4);
    set_hitbox_dimensions(32, 32);
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

    // Update sprite animation state based on current movement/state
    update_sprite_state();
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
        // Only set horizontal velocity - gravity handles vertical
        velocity.x = direction.x * speed;

        // Jump if target is above and we're on the ground
        if (target.y < coords.y - 10 && on_ground)
        {
            jump();
        }
    }
}

void Enemy::move_away_from(const glm::ivec2 &target, float delta_time)
{
    glm::vec2 direction = glm::vec2(coords - target); // Reversed direction
    float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (dist > 0.1f)
    {
        direction /= dist;
        // Only set horizontal velocity - gravity handles vertical
        velocity.x = direction.x * speed * 1.2f; // Run faster when fleeing

        // Jump if we need to go up while fleeing and we're on the ground
        if (direction.y < -0.3f && on_ground)
        {
            jump();
        }
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

float Enemy::get_attack_damage() const
{
    return attack_damage;
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

// ==================== Devushki (NPC) ====================

Devushki::Devushki()
{
    type = Entity_Type::DEVUSHKI;
    max_healthpoints = 80.0f;
    healthpoints = 80.0f;
    speed = 45.0f;
    set_hitbox_dimensions(28, 40);
    npc_ai_state = NPC_AI_State::IDLE;
}

Devushki::Devushki(std::string name, glm::vec2 coords)
    : Devushki()
{
    this->name = name;
    this->coords = coords;
    this->home_position = coords;
}

void Devushki::update(float delta_time)
{
    if (!is_alive)
        return;

    state_timer += delta_time;

    switch (npc_ai_state)
    {
    case NPC_AI_State::IDLE:
        state_idle(delta_time);
        break;
    case NPC_AI_State::FOLLOW:
        state_follow(delta_time);
        break;
    case NPC_AI_State::WANDER:
        state_wander(delta_time);
        break;
    case NPC_AI_State::INTERACT:
        state_interact(delta_time);
        break;
    }

    update_physics(delta_time);
    calculate_hitbox();
}

// ==================== Devushki State Handlers ====================

void Devushki::state_idle(float delta_time)
{
    velocity *= 0.9f;
    state = Entity_States::STILL;

    // Check if player is nearby - start following
    if (is_in_follow_range(target_position))
    {
        transition_to(NPC_AI_State::FOLLOW);
        return;
    }

    // Column objective behavior: do not wander away from home while player is out of range.
    if (state_timer >= idle_duration)
    {
        state_timer = 0.0f;
    }
}

void Devushki::state_follow(float delta_time)
{
    state = Entity_States::WALKING;

    float dist = distance_to(target_position);

    // Player went too far - lose interest
    if (dist > lose_interest_range)
    {
        transition_to(NPC_AI_State::IDLE);
        return;
    }

    // Close enough to player - stop and interact
    if (dist <= stop_follow_range)
    {
        velocity *= 0.5f;
        transition_to(NPC_AI_State::INTERACT);
        return;
    }

    // Follow the player
    move_towards(target_position, delta_time);
}

void Devushki::state_wander(float delta_time)
{
    state = Entity_States::WALKING;

    // Check if player is nearby - interrupt wander
    if (is_in_follow_range(target_position))
    {
        transition_to(NPC_AI_State::FOLLOW);
        return;
    }

    float dist = distance_to(wander_target);

    if (dist < 10.0f)
    {
        transition_to(NPC_AI_State::IDLE);
        return;
    }

    move_towards(wander_target, delta_time);

    if (state_timer >= wander_duration)
    {
        transition_to(NPC_AI_State::IDLE);
    }
}

void Devushki::state_interact(float delta_time)
{
    velocity *= 0.8f;
    state = Entity_States::STILL;

    float dist = distance_to(target_position);

    // Player moved away
    if (dist > stop_follow_range * 2.0f)
    {
        transition_to(NPC_AI_State::FOLLOW);
        return;
    }

    if (state_timer >= interact_duration)
    {
        transition_to(NPC_AI_State::IDLE);
    }
}

void Devushki::transition_to(NPC_AI_State new_state)
{
    if (npc_ai_state == new_state)
        return;

    npc_ai_state = new_state;
    state_timer = 0.0f;

    switch (new_state)
    {
    case NPC_AI_State::IDLE:
        velocity = {0.0f, 0.0f};
        break;
    case NPC_AI_State::FOLLOW:
        break;
    case NPC_AI_State::WANDER:
        wander_target = get_random_wander_point();
        break;
    case NPC_AI_State::INTERACT:
        velocity = {0.0f, 0.0f};
        break;
    }
}

void Devushki::move_towards(const glm::ivec2 &target, float delta_time)
{
    glm::vec2 direction = glm::vec2(target - coords);
    float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (dist > 1.0f)
    {
        direction /= dist;
        // Only set horizontal velocity - gravity handles vertical
        float desired_vx = direction.x * speed * follow_speed_multiplier;

        // Avoid stepping off edges when standing on narrow supports (like devushki columns).
        if (on_ground && world_ref && !noclip && std::abs(desired_vx) > 0.01f)
        {
            const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
            const int dir = (desired_vx > 0.0f) ? 1 : -1;
            const int probe_x = coords.x + dir * (hitbox_dimensions_half.x + ps);
            const int support_y = coords.y + hitbox_dimensions_half.y + ps;

            const bool support_ahead =
                is_solid_at(probe_x, support_y) ||
                is_solid_at(probe_x - hitbox_dimensions_half.x / 2, support_y) ||
                is_solid_at(probe_x + hitbox_dimensions_half.x / 2, support_y);

            if (!support_ahead)
            {
                velocity.x = 0.0f;
                return;
            }
        }

        velocity.x = desired_vx;

        // Jump if target is above and we're on the ground
        if (target.y < coords.y - 10 && on_ground)
        {
            jump();
        }
    }
}

float Devushki::distance_to(const glm::ivec2 &target) const
{
    glm::vec2 diff = glm::vec2(target - coords);
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

glm::ivec2 Devushki::get_random_wander_point() const
{
    float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
    float distance = 40.0f + static_cast<float>(rand()) / RAND_MAX * 80.0f;

    return glm::ivec2{
        home_position.x + static_cast<int>(std::cos(angle) * distance),
        home_position.y + static_cast<int>(std::sin(angle) * distance)};
}

void Devushki::set_target(const glm::ivec2 &target)
{
    target_position = target;
}

void Devushki::set_home_position(const glm::ivec2 &home)
{
    home_position = home;
}

void Devushki::set_follow_range(float range)
{
    follow_range = range;
}

NPC_AI_State Devushki::get_npc_ai_state() const
{
    return npc_ai_state;
}

const char *Devushki::get_npc_ai_state_name() const
{
    switch (npc_ai_state)
    {
    case NPC_AI_State::IDLE:
        return "IDLE";
    case NPC_AI_State::FOLLOW:
        return "FOLLOW";
    case NPC_AI_State::WANDER:
        return "WANDER";
    case NPC_AI_State::INTERACT:
        return "INTERACT";
    default:
        return "UNKNOWN";
    }
}

bool Devushki::is_in_follow_range(const glm::ivec2 &target) const
{
    return distance_to(target) <= follow_range;
}

// ==================== Boss ====================

Boss::Boss()
{
    type = Entity_Type::BOSS;
    max_healthpoints = 500.0f;
    healthpoints = 500.0f;
    speed = 40.0f;
    set_hitbox_dimensions(48, 56);
    boss_ai_state = Boss_AI_State::IDLE;
}

Boss::Boss(std::string name, glm::vec2 coords)
    : Boss()
{
    this->name = name;
    this->coords = coords;
    this->home_position = coords;
}

void Boss::update(float delta_time)
{
    if (!is_alive)
    {
        boss_ai_state = Boss_AI_State::DEAD;
        return;
    }

    time_since_attack += delta_time;
    time_since_slam += delta_time;
    time_since_fireball += delta_time;
    time_since_teleport += delta_time;
    state_timer += delta_time;

    // Check enrage condition
    if (should_enrage() && !is_enraged)
    {
        is_enraged = true;
        transition_to(Boss_AI_State::ENRAGE);
    }

    switch (boss_ai_state)
    {
    case Boss_AI_State::IDLE:
        state_idle(delta_time);
        break;
    case Boss_AI_State::CHASE:
        state_chase(delta_time);
        break;
    case Boss_AI_State::ATTACK:
        state_attack(delta_time);
        break;
    case Boss_AI_State::SLAM:
        state_slam(delta_time);
        break;
    case Boss_AI_State::ENRAGE:
        state_enrage(delta_time);
        break;
    case Boss_AI_State::DEAD:
        velocity = {0.0f, 0.0f};
        break;
    }

    update_physics(delta_time);
    calculate_hitbox();
}

// ==================== Boss State Handlers ====================

void Boss::state_idle(float delta_time)
{
    velocity *= 0.9f;
    state = Entity_States::STILL;

    if (is_in_detection_range(target_position))
    {
        transition_to(Boss_AI_State::CHASE);
    }
}

void Boss::state_chase(float delta_time)
{
    state = Entity_States::WALKING;

    float dist = distance_to(target_position);

    // Lost interest
    if (dist > lose_interest_range)
    {
        transition_to(Boss_AI_State::IDLE);
        return;
    }

    // In slam range and slam is ready - slam attack
    if (dist <= slam_range && can_slam() && is_enraged)
    {
        transition_to(Boss_AI_State::SLAM);
        return;
    }

    // In attack range - attack
    if (dist <= attack_range)
    {
        transition_to(Boss_AI_State::ATTACK);
        return;
    }

    if (dist <= fire_attack_range && can_fireball())
    {
        queue_fireball();
    }

    // Chase the target using unified movement
    move_towards(target_position, delta_time);
}

void Boss::state_attack(float delta_time)
{
    state = Entity_States::STILL;
    velocity *= 0.3f;

    float dist = distance_to(target_position);

    if (dist > attack_range * 1.3f)
    {
        transition_to(Boss_AI_State::CHASE);
        return;
    }

    if (can_attack())
    {
        time_since_attack = 0.0f;
        state = Entity_States::HIT;
        // TODO: Deal damage through entity_manager

        if (can_fireball())
        {
            queue_fireball();
        }
    }
}

void Boss::state_slam(float delta_time)
{
    state = Entity_States::HIT;
    velocity = {0.0f, 0.0f};

    // Slam is a brief action
    if (state_timer >= 0.5f)
    {
        time_since_slam = 0.0f;
        // TODO: Deal AoE damage through entity_manager
        transition_to(Boss_AI_State::CHASE);
    }
}

void Boss::state_enrage(float delta_time)
{
    // Brief enrage animation, then go back to chasing
    velocity = {0.0f, 0.0f};
    state = Entity_States::HIT;

    if (state_timer >= 1.0f)
    {
        transition_to(Boss_AI_State::CHASE);
    }

    if (distance_to(target_position) <= fire_attack_range * 1.2f && can_fireball())
    {
        queue_fireball();
    }
}

void Boss::transition_to(Boss_AI_State new_state)
{
    if (boss_ai_state == new_state)
        return;

    previous_boss_ai_state = boss_ai_state;
    boss_ai_state = new_state;
    state_timer = 0.0f;

    switch (new_state)
    {
    case Boss_AI_State::IDLE:
        velocity = {0.0f, 0.0f};
        break;
    case Boss_AI_State::CHASE:
        break;
    case Boss_AI_State::ATTACK:
        velocity = {0.0f, 0.0f};
        break;
    case Boss_AI_State::SLAM:
        velocity = {0.0f, 0.0f};
        break;
    case Boss_AI_State::ENRAGE:
        velocity = {0.0f, 0.0f};
        break;
    case Boss_AI_State::DEAD:
        velocity = {0.0f, 0.0f};
        break;
    }
}

bool Boss::should_enrage() const
{
    return (healthpoints / max_healthpoints) <= enrage_health_threshold;
}

bool Boss::can_attack() const
{
    return time_since_attack >= (is_enraged ? attack_cooldown * 0.7f : attack_cooldown);
}

bool Boss::can_slam() const
{
    return time_since_slam >= slam_cooldown;
}

bool Boss::can_fireball() const
{
    const float effective_cooldown = is_enraged ? fireball_cooldown * 0.7f : fireball_cooldown;
    return time_since_fireball >= effective_cooldown && pending_fireball_shots.size() < 18;
}

void Boss::queue_fireball()
{
    if (is_enraged)
    {
        switch (fire_pattern_cycle % 3)
        {
        case 0:
            queue_fireball_fan(3, 0.55f, 1.08f);
            break;
        case 1:
            queue_fireball_spiral(5, 0.34f, 0.92f);
            break;
        default:
            queue_fireball_fan(5, 1.02f, 1.0f);
            break;
        }
    }
    else
    {
        if (fire_pattern_cycle % 4 == 3)
            queue_fireball_fan(2, 0.22f, 0.95f);
        else
            queue_fireball_fan(1, 0.0f, 1.0f);
    }

    ++fire_pattern_cycle;
    time_since_fireball = 0.0f;
}

void Boss::queue_fireball_shot(const glm::vec2 &direction, float damage_scale, Particle_Type payload)
{
    const glm::vec2 dir = safe_normalize_vec2(direction, glm::vec2(1.0f, 0.0f));

    PendingFireballShot shot;
    shot.origin = glm::vec2(coords);
    shot.velocity = dir * fireball_speed;
    shot.damage = fireball_damage * damage_scale;
    if (is_enraged)
    {
        shot.damage *= 1.2f;
    }
    shot.payload = payload;
    pending_fireball_shots.push_back(shot);
}

void Boss::queue_fireball_fan(int shot_count, float total_arc_radians, float damage_scale)
{
    if (shot_count <= 0)
        return;

    const glm::vec2 aim_dir = safe_normalize_vec2(glm::vec2(target_position - coords), glm::vec2(1.0f, 0.0f));

    if (shot_count == 1)
    {
        queue_fireball_shot(aim_dir, damage_scale);
        return;
    }

    const float start_angle = -0.5f * total_arc_radians;
    const float angle_step = total_arc_radians / static_cast<float>(shot_count - 1);

    for (int i = 0; i < shot_count; ++i)
    {
        const float angle = start_angle + angle_step * static_cast<float>(i);
        queue_fireball_shot(rotate_vec2(aim_dir, angle), damage_scale);
    }
}

void Boss::queue_fireball_spiral(int shot_count, float angle_step_radians, float damage_scale)
{
    if (shot_count <= 0)
        return;

    const glm::vec2 aim_dir = safe_normalize_vec2(glm::vec2(target_position - coords), glm::vec2(1.0f, 0.0f));

    for (int i = 0; i < shot_count; ++i)
    {
        const float angle = spiral_seed_angle + angle_step_radians * static_cast<float>(i);
        queue_fireball_shot(rotate_vec2(aim_dir, angle), damage_scale);
    }

    spiral_seed_angle += angle_step_radians * 0.7f;
    const float two_pi = 6.28318530718f;
    if (spiral_seed_angle > two_pi)
    {
        spiral_seed_angle = std::fmod(spiral_seed_angle, two_pi);
    }
}

glm::ivec2 Boss::choose_teleport_position_around_target(const glm::vec2 &preferred_dir) const
{
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist_dist(teleport_min_distance, teleport_max_distance);
    std::uniform_real_distribution<float> jitter(-0.7f, 0.7f);
    std::uniform_int_distribution<int> side_dist(0, 1);

    glm::vec2 dir = preferred_dir;
    const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len <= 0.001f)
    {
        dir = glm::vec2(1.0f, 0.0f);
    }
    else
    {
        dir /= len;
    }

    const glm::vec2 center = glm::vec2(target_position);

    for (int i = 0; i < 10; ++i)
    {
        const float side_mul = side_dist(rng) == 0 ? -1.0f : 1.0f;
        const float angle = jitter(rng) * side_mul;
        const float cos_a = std::cos(angle);
        const float sin_a = std::sin(angle);

        glm::vec2 rotated_dir(
            dir.x * cos_a - dir.y * sin_a,
            dir.x * sin_a + dir.y * cos_a);

        const float radius = dist_dist(rng);
        glm::ivec2 candidate = glm::ivec2(center + rotated_dir * radius);
        candidate = find_valid_spawn_position(candidate, 280);

        const glm::vec2 delta = glm::vec2(candidate - coords);
        const float move_dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (move_dist < 60.0f)
            continue;

        if (!check_collision_at(candidate))
            return candidate;
    }

    return coords;
}

void Boss::move_towards(const glm::ivec2 &target, float delta_time)
{
    glm::vec2 direction = glm::vec2(target - coords);
    float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (dist > 1.0f)
    {
        direction /= dist;
        float current_speed = is_enraged ? speed * enrage_speed_multiplier : speed;
        // Only set horizontal velocity - gravity handles vertical
        velocity.x = direction.x * current_speed;

        // Jump if target is above and we're on the ground
        if (target.y < coords.y - 10 && on_ground)
        {
            jump();
        }
    }
}

float Boss::distance_to(const glm::ivec2 &target) const
{
    glm::vec2 diff = glm::vec2(target - coords);
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

void Boss::set_target(const glm::ivec2 &target)
{
    target_position = target;
}

void Boss::set_home_position(const glm::ivec2 &home)
{
    home_position = home;
}

void Boss::set_detection_range(float range)
{
    detection_range = range;
}

void Boss::set_attack_range(float range)
{
    attack_range = range;
}

void Boss::set_attack_damage(float damage)
{
    attack_damage = damage;
}

void Boss::set_slam_damage(float damage)
{
    slam_damage = damage;
}

float Boss::get_attack_damage() const
{
    return is_enraged ? attack_damage * enrage_damage_multiplier : attack_damage;
}

Boss_AI_State Boss::get_boss_ai_state() const
{
    return boss_ai_state;
}

const char *Boss::get_boss_ai_state_name() const
{
    switch (boss_ai_state)
    {
    case Boss_AI_State::IDLE:
        return "IDLE";
    case Boss_AI_State::CHASE:
        return "CHASE";
    case Boss_AI_State::ATTACK:
        return "ATTACK";
    case Boss_AI_State::SLAM:
        return "SLAM";
    case Boss_AI_State::ENRAGE:
        return "ENRAGE";
    case Boss_AI_State::DEAD:
        return "DEAD";
    default:
        return "UNKNOWN";
    }
}

bool Boss::is_in_attack_range(const glm::ivec2 &target) const
{
    return distance_to(target) <= attack_range;
}

bool Boss::is_in_detection_range(const glm::ivec2 &target) const
{
    return distance_to(target) <= detection_range;
}

bool Boss::get_is_enraged() const
{
    return is_enraged;
}

bool Boss::consume_pending_fireball(glm::vec2 &out_origin, glm::vec2 &out_velocity,
                                    float &out_damage, Particle_Type &out_payload)
{
    if (pending_fireball_shots.empty())
        return false;

    const PendingFireballShot shot = pending_fireball_shots.front();
    pending_fireball_shots.erase(pending_fireball_shots.begin());

    out_origin = shot.origin;
    out_velocity = shot.velocity;
    out_damage = shot.damage;
    out_payload = shot.payload;
    return true;
}

bool Boss::try_teleport_dodge_from(const glm::ivec2 &threat_pos, const glm::vec2 &threat_velocity)
{
    if (!is_alive || boss_ai_state == Boss_AI_State::DEAD)
        return false;
    if (time_since_teleport < teleport_cooldown)
        return false;
    if (!is_in_detection_range(target_position))
        return false;

    const glm::vec2 to_boss = glm::vec2(coords - threat_pos);
    const float threat_dist = std::sqrt(to_boss.x * to_boss.x + to_boss.y * to_boss.y);
    if (threat_dist > 260.0f)
        return false;

    glm::vec2 preferred_dir(0.0f, 0.0f);
    const float vel_len = std::sqrt(threat_velocity.x * threat_velocity.x + threat_velocity.y * threat_velocity.y);
    if (vel_len > 0.001f)
    {
        const glm::vec2 vel_norm = threat_velocity / vel_len;
        preferred_dir = glm::vec2(-vel_norm.y, vel_norm.x);
    }
    else if (threat_dist > 0.001f)
    {
        preferred_dir = to_boss / threat_dist;
    }
    else
    {
        preferred_dir = glm::vec2(1.0f, 0.0f);
    }

    const glm::ivec2 teleport_pos = choose_teleport_position_around_target(preferred_dir);
    if (teleport_pos == coords)
        return false;

    set_position(teleport_pos);
    velocity = {0.0f, 0.0f};
    state = Entity_States::HIT;
    time_since_teleport = 0.0f;

    if (boss_ai_state != Boss_AI_State::DEAD)
    {
        transition_to(Boss_AI_State::CHASE);
    }

    return true;
}
