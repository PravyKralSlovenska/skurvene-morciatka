#include "engine/particle/falling_sand_simulation.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/particle/particle.hpp"
#include "others/utils.hpp"

Falling_Sand_Simulation::Falling_Sand_Simulation()
{
    // Initialize random number generator with random seed
    std::random_device rd;
    rng.seed(rd());
}

void Falling_Sand_Simulation::set_world(World *world)
{
    this->world = world;
}

Chunk *Falling_Sand_Simulation::get_chunk_cached(const glm::ivec2 &coords)
{
    for (auto &entry : chunk_cache)
    {
        if (entry.coords == coords && entry.chunk != nullptr)
        {
            return entry.chunk;
        }
    }
    Chunk *chunk = world->get_chunk(coords);
    chunk_cache[cache_index] = {coords, chunk};
    cache_index = (cache_index + 1) % chunk_cache.size();
    return chunk;
}

glm::ivec2 Falling_Sand_Simulation::local_to_world(const glm::ivec2 &chunk_coords, int x, int y)
{
    glm::ivec2 chunk_dim = world->get_chunk_dimensions();
    return glm::ivec2(chunk_coords.x * chunk_dim.x + x, chunk_coords.y * chunk_dim.y + y);
}

glm::ivec2 Falling_Sand_Simulation::world_to_chunk(const glm::ivec2 &world_pos)
{
    glm::ivec2 chunk_dim = world->get_chunk_dimensions();
    int chunk_x = (world_pos.x >= 0) ? (world_pos.x / chunk_dim.x) : ((world_pos.x - chunk_dim.x + 1) / chunk_dim.x);
    int chunk_y = (world_pos.y >= 0) ? (world_pos.y / chunk_dim.y) : ((world_pos.y - chunk_dim.y + 1) / chunk_dim.y);
    return glm::ivec2(chunk_x, chunk_y);
}

glm::ivec2 Falling_Sand_Simulation::world_to_local(const glm::ivec2 &world_pos)
{
    glm::ivec2 chunk_dim = world->get_chunk_dimensions();
    int local_x = ((world_pos.x % chunk_dim.x) + chunk_dim.x) % chunk_dim.x;
    int local_y = ((world_pos.y % chunk_dim.y) + chunk_dim.y) % chunk_dim.y;
    return glm::ivec2(local_x, local_y);
}

WorldCell *Falling_Sand_Simulation::get_cell_at(const glm::ivec2 &chunk_coords, int x, int y)
{
    // Calculate actual chunk and local coordinates
    glm::ivec2 world_pos = local_to_world(chunk_coords, x, y);
    glm::ivec2 actual_chunk = world_to_chunk(world_pos);
    glm::ivec2 local_pos = world_to_local(world_pos);

    Chunk *chunk = get_chunk_cached(actual_chunk);
    if (!chunk)
        return nullptr;

    return chunk->get_worldcell(local_pos.x, local_pos.y);
}

WorldCell *Falling_Sand_Simulation::get_cell_at_fast(const glm::ivec2 &chunk_coords, int x, int y, Chunk *current_chunk)
{
    glm::ivec2 chunk_dim = world->get_chunk_dimensions();
    if (x >= 0 && x < chunk_dim.x && y >= 0 && y < chunk_dim.y)
    {
        return current_chunk->get_worldcell(x, y);
    }
    return get_cell_at(chunk_coords, x, y);
}

WorldCell *Falling_Sand_Simulation::get_cell_at_world_pos(const glm::ivec2 &world_cell_pos)
{
    glm::ivec2 chunk_coords = world_to_chunk(world_cell_pos);
    glm::ivec2 local_pos = world_to_local(world_cell_pos);

    Chunk *chunk = get_chunk_cached(chunk_coords);
    if (!chunk)
        return nullptr;

    return chunk->get_worldcell(local_pos.x, local_pos.y);
}

bool Falling_Sand_Simulation::is_valid_position(const glm::ivec2 &chunk_coords, int x, int y)
{
    return get_cell_at(chunk_coords, x, y) != nullptr;
}

bool Falling_Sand_Simulation::spread_fire_to_adjacent_wood(const glm::ivec2 &chunk_coords, int x, int y)
{
    WorldCell *fire_cell = get_cell_at(chunk_coords, x, y);
    if (!fire_cell || fire_cell->particle.type != Particle_Type::FIRE)
        return false;

    const glm::ivec2 offsets[] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

    std::vector<glm::ivec2> wood_neighbors;
    wood_neighbors.reserve(8);

    for (const auto &offset : offsets)
    {
        glm::ivec2 neighbor_world_pos = local_to_world(chunk_coords, x + offset.x, y + offset.y);
        WorldCell *neighbor = get_cell_at_world_pos(neighbor_world_pos);

        if (neighbor && neighbor->particle.type == Particle_Type::WOOD)
        {
            wood_neighbors.push_back(neighbor_world_pos);
        }
    }

    if (wood_neighbors.empty())
        return false;

    std::uniform_int_distribution<int> pick(0, static_cast<int>(wood_neighbors.size()) - 1);
    glm::ivec2 target_world_pos = wood_neighbors[pick(rng)];
    WorldCell *target = get_cell_at_world_pos(target_world_pos);
    if (!target)
        return false;

    target->particle = create_fire(false);
    target->particle.mark_updated();

    Chunk *fire_chunk = get_chunk_cached(world_to_chunk(local_to_world(chunk_coords, x, y)));
    Chunk *target_chunk = get_chunk_cached(world_to_chunk(target_world_pos));
    if (fire_chunk)
        fire_chunk->mark_dirty();
    if (target_chunk)
        target_chunk->mark_dirty();

    return true;
}

bool Falling_Sand_Simulation::try_swap(WorldCell *from, WorldCell *to)
{
    if (!from || !to)
        return false;

    // Can't swap with static particles (terrain)
    if (to->particle.flags.is_static)
        return false;

    // Can't move if already updated this frame
    if (from->particle.flags.is_updated)
        return false;

    // Empty target - just move
    if (to->particle.type == Particle_Type::EMPTY)
    {
        std::swap(from->particle, to->particle);
        to->particle.mark_updated();
        from->particle.reset_update_flag();
        return true;
    }

    // Density-based displacement (heavier particles sink)
    if (from->particle.can_displace(to->particle))
    {
        std::swap(from->particle, to->particle);
        to->particle.mark_updated();
        // Don't mark the displaced particle as updated so it can move this frame
        return true;
    }

    return false;
}

bool Falling_Sand_Simulation::try_move(const glm::ivec2 &from_chunk, int from_x, int from_y,
                                       const glm::ivec2 &to_chunk, int to_x, int to_y)
{
    WorldCell *from_cell = get_cell_at(from_chunk, from_x, from_y);
    WorldCell *to_cell = get_cell_at(to_chunk, to_x, to_y);

    bool success = try_swap(from_cell, to_cell);

    if (success)
    {
        // Mark both chunks as dirty for GPU re-upload
        // Convert local coords (which might be out of bounds) to actual world coords
        glm::ivec2 from_world = local_to_world(from_chunk, from_x, from_y);
        glm::ivec2 to_world = local_to_world(to_chunk, to_x, to_y);

        glm::ivec2 actual_from_chunk = world_to_chunk(from_world);
        glm::ivec2 actual_to_chunk = world_to_chunk(to_world);

        Chunk *chunk_from = get_chunk_cached(actual_from_chunk);
        Chunk *chunk_to = get_chunk_cached(actual_to_chunk);

        if (chunk_from)
            chunk_from->mark_dirty();
        if (chunk_to && chunk_to != chunk_from)
            chunk_to->mark_dirty();
    }

    return success;
}

void Falling_Sand_Simulation::apply_gravity(Particle &particle, float delta_time)
{
    if (particle.flags.is_static || particle.type == Particle_Type::EMPTY)
        return;

    // Apply gravity based on particle state
    float gravity_multiplier = 1.0f;

    switch (particle.state)
    {
    case Particle_State::SOLID:
        gravity_multiplier = 1.0f;
        break;
    case Particle_State::LIQUID:
        gravity_multiplier = 0.9f; // Liquids fall slightly slower (viscosity)
        break;
    case Particle_State::GAS:
        gravity_multiplier = -0.5f; // Gases rise
        break;
    default:
        break;
    }

    // Set gravitational acceleration
    particle.physics.acceleration.y = GRAVITY * gravity_multiplier;

    // Apply acceleration to velocity (v = v + a * dt)
    particle.physics.velocity.x += particle.physics.acceleration.x * delta_time;
    particle.physics.velocity.y += particle.physics.acceleration.y * delta_time;

    // Apply friction to horizontal velocity
    particle.physics.velocity.x *= FRICTION;

    if (particle.type == Particle_Type::FIRE)
    {
        // Flamethrower behavior: strong forward momentum that rapidly degrades,
        // while buoyancy increasingly bends the trajectory upward.
        particle.physics.velocity.x *= 0.98f;
        const float horizontal_energy = std::abs(particle.physics.velocity.x);
        particle.physics.velocity.y -= (1.0f + horizontal_energy * 0.05f) * delta_time;
    }

    // Reset acceleration after applying (forces are applied each frame)
    particle.physics.acceleration = {0.0f, 0.0f};

    // Clamp to max velocity
    particle.physics.velocity.y = std::clamp(particle.physics.velocity.y, -MAX_VELOCITY, MAX_VELOCITY);
    particle.physics.velocity.x = std::clamp(particle.physics.velocity.x, -MAX_VELOCITY, MAX_VELOCITY);

    // Zero out tiny velocities to prevent jitter
    if (std::abs(particle.physics.velocity.x) < 0.1f)
        particle.physics.velocity.x = 0.0f;
}

void Falling_Sand_Simulation::update_solid(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell, Chunk *current_chunk)
{
    if (!cell || cell->particle.flags.is_updated)
        return;

    Particle &particle = cell->particle;

    // Static solids don't move
    if (particle.flags.is_static || particle.move == Particle_Movement::STATIC)
        return;

    if (particle.type == Particle_Type::EMPTY)
        return;

    // Calculate how many cells to move based on velocity
    int vy_steps = std::max(1, static_cast<int>(std::abs(particle.physics.velocity.y)));
    int vx_steps = static_cast<int>(std::abs(particle.physics.velocity.x));
    int total_steps = std::max(vy_steps, vx_steps);

    bool moved = false;
    bool blocked_vertical = false;

    for (int step = 0; step < total_steps && !blocked_vertical; step++)
    {
        // Try to move down first (gravity)
        if (try_move(chunk_coords, x, y, chunk_coords, x, y + 1))
        {
            y += 1; // Update position for next iteration
            moved = true;
            continue;
        }

        // If can't move down, try diagonal (with randomization to prevent bias)
        bool try_left_first = (rng() % 2 == 0) ^ process_left_first;

        // Add horizontal velocity bias to diagonal choice
        if (particle.physics.velocity.x > 0.5f)
            try_left_first = false; // Prefer right
        else if (particle.physics.velocity.x < -0.5f)
            try_left_first = true; // Prefer left

        int dir1 = try_left_first ? -1 : 1;
        int dir2 = try_left_first ? 1 : -1;

        // Try first diagonal
        if (try_move(chunk_coords, x, y, chunk_coords, x + dir1, y + 1))
        {
            x += dir1;
            y += 1;
            // Transfer some vertical momentum to horizontal when sliding
            WorldCell *new_cell = get_cell_at_fast(chunk_coords, x, y, current_chunk);
            if (new_cell)
            {
                new_cell->particle.physics.velocity.x += dir1 * std::abs(particle.physics.velocity.y) * 0.3f;
            }
            moved = true;
            continue;
        }

        // Try second diagonal
        if (try_move(chunk_coords, x, y, chunk_coords, x + dir2, y + 1))
        {
            x += dir2;
            y += 1;
            WorldCell *new_cell = get_cell_at_fast(chunk_coords, x, y, current_chunk);
            if (new_cell)
            {
                new_cell->particle.physics.velocity.x += dir2 * std::abs(particle.physics.velocity.y) * 0.3f;
            }
            moved = true;
            continue;
        }

        // Blocked - lose vertical velocity, transfer to horizontal
        blocked_vertical = true;

        // Can't move anymore
        break;
    }

    // If we didn't move at all, mark the original cell as updated
    if (!moved)
    {
        cell->particle.mark_updated();
    }
}

void Falling_Sand_Simulation::update_liquid(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell, Chunk *current_chunk)
{
    if (!cell || cell->particle.flags.is_updated)
        return;

    Particle &particle = cell->particle;

    if (particle.flags.is_static)
        return;

    if (particle.type == Particle_Type::EMPTY)
        return;

    // Calculate movement steps based on velocity
    int vy_steps = std::max(1, static_cast<int>(std::abs(particle.physics.velocity.y)));
    int vx_steps = static_cast<int>(std::abs(particle.physics.velocity.x));

    bool moved = false;

    // First, handle vertical movement with velocity
    for (int step = 0; step < vy_steps; step++)
    {
        // Try to move down
        if (try_move(chunk_coords, x, y, chunk_coords, x, y + 1))
        {
            y += 1;
            moved = true;
            continue;
        }

        // Try diagonal down
        bool try_left_first = (rng() % 2 == 0) ^ process_left_first;

        // Bias by horizontal velocity
        if (particle.physics.velocity.x > 0.5f)
            try_left_first = false;
        else if (particle.physics.velocity.x < -0.5f)
            try_left_first = true;

        int dir1 = try_left_first ? -1 : 1;
        int dir2 = try_left_first ? 1 : -1;

        if (try_move(chunk_coords, x, y, chunk_coords, x + dir1, y + 1))
        {
            x += dir1;
            y += 1;
            // Transfer vertical momentum to horizontal
            WorldCell *new_cell = get_cell_at_fast(chunk_coords, x, y, current_chunk);
            if (new_cell)
            {
                new_cell->particle.physics.velocity.x += dir1 * std::abs(particle.physics.velocity.y) * 0.5f;
            }
            moved = true;
            continue;
        }

        if (try_move(chunk_coords, x, y, chunk_coords, x + dir2, y + 1))
        {
            x += dir2;
            y += 1;
            WorldCell *new_cell = get_cell_at_fast(chunk_coords, x, y, current_chunk);
            if (new_cell)
            {
                new_cell->particle.physics.velocity.x += dir2 * std::abs(particle.physics.velocity.y) * 0.5f;
            }
            moved = true;
            continue;
        }

        // Can't move down anymore - convert remaining vertical velocity to horizontal spread
        WorldCell *current = get_cell_at_fast(chunk_coords, x, y, current_chunk);
        if (current)
        {
            float remaining_vy = current->particle.physics.velocity.y;
            current->particle.physics.velocity.y *= BOUNCE; // Lose energy on impact
            // Random horizontal scatter
            current->particle.physics.velocity.x += ((rng() % 2) ? 1 : -1) * remaining_vy * 0.4f;
        }
        break;
    }

    // Now handle horizontal spreading (based on velocity + dispersion)
    int dispersion = particle.physics.dispersion_rate + vx_steps;
    int preferred_dir = (particle.physics.velocity.x >= 0) ? 1 : -1;

    for (int d = 1; d <= dispersion; d++)
    {
        // Try preferred direction first (based on velocity)
        if (try_move(chunk_coords, x, y, chunk_coords, x + preferred_dir * d, y))
        {
            x += preferred_dir * d;
            moved = true;
            break;
        }
        // Try opposite direction
        if (try_move(chunk_coords, x, y, chunk_coords, x - preferred_dir * d, y))
        {
            x -= preferred_dir * d;
            // Reverse velocity if we went opposite way
            WorldCell *new_cell = get_cell_at_fast(chunk_coords, x, y, current_chunk);
            if (new_cell)
            {
                new_cell->particle.physics.velocity.x *= -0.5f;
            }
            moved = true;
            break;
        }
    }

    // Couldn't move at all - mark as updated and reduce velocity
    if (!moved)
    {
        cell->particle.physics.velocity *= 0.5f;
        cell->particle.mark_updated();
    }
}

void Falling_Sand_Simulation::update_gas(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell, Chunk *current_chunk)
{
    if (!cell || cell->particle.flags.is_updated)
        return;

    Particle &particle = cell->particle;

    if (particle.flags.is_static)
        return;

    if (particle.type == Particle_Type::EMPTY)
        return;

    // Decrease lifetime
    if (particle.lifetime > 0)
    {
        particle.lifetime--;
        if (particle.lifetime == 0)
        {
            // Fire cools into smoke, other gases dissipate.
            if (particle.type == Particle_Type::FIRE)
            {
                particle = create_smoke(false);
            }
            else
            {
                particle = Particle(); // Reset to empty
            }
            return;
        }
    }

    const int vx_steps = static_cast<int>(std::abs(particle.physics.velocity.x));
    const int vy_steps = static_cast<int>(std::abs(particle.physics.velocity.y));
    const int total_steps = std::max(1, std::max(vx_steps, vy_steps));

    int horizontal_dir = (particle.physics.velocity.x >= 0.0f) ? 1 : -1;
    int vertical_dir = (particle.physics.velocity.y >= 0.0f) ? 1 : -1;

    bool moved_any = false;
    for (int step = 0; step < total_steps; ++step)
    {
        bool moved_this_step = false;

        float horizontal_bias = 1.0f;
        if (particle.type == Particle_Type::FIRE)
        {
            horizontal_bias = 3.0f;
        }
        const bool prefer_horizontal = std::abs(particle.physics.velocity.x) * horizontal_bias >=
                                       std::abs(particle.physics.velocity.y);

        if (prefer_horizontal)
        {
            if (step < vx_steps && try_move(chunk_coords, x, y, chunk_coords, x + horizontal_dir, y))
            {
                x += horizontal_dir;
                moved_this_step = true;
            }
            else if (step < vy_steps && try_move(chunk_coords, x, y, chunk_coords, x, y + vertical_dir))
            {
                y += vertical_dir;
                moved_this_step = true;
            }
            else if (step < vx_steps && step < vy_steps &&
                     try_move(chunk_coords, x, y, chunk_coords, x + horizontal_dir, y + vertical_dir))
            {
                x += horizontal_dir;
                y += vertical_dir;
                moved_this_step = true;
            }
        }
        else
        {
            if (step < vy_steps && try_move(chunk_coords, x, y, chunk_coords, x, y + vertical_dir))
            {
                y += vertical_dir;
                moved_this_step = true;
            }
            else if (step < vx_steps && try_move(chunk_coords, x, y, chunk_coords, x + horizontal_dir, y))
            {
                x += horizontal_dir;
                moved_this_step = true;
            }
            else if (step < vx_steps && step < vy_steps &&
                     try_move(chunk_coords, x, y, chunk_coords, x + horizontal_dir, y + vertical_dir))
            {
                x += horizontal_dir;
                y += vertical_dir;
                moved_this_step = true;
            }
        }

        if (!moved_this_step)
            break;

        moved_any = true;
    }

    if (moved_any)
        return;

    // Fallback diffusion when velocity-driven movement is blocked.
    bool try_left_first = (rng() % 2 == 0) ^ process_left_first;
    int dir1 = try_left_first ? -1 : 1;
    int dir2 = try_left_first ? 1 : -1;

    if (try_move(chunk_coords, x, y, chunk_coords, x, y - 1))
        return;
    if (try_move(chunk_coords, x, y, chunk_coords, x + dir1, y - 1))
        return;
    if (try_move(chunk_coords, x, y, chunk_coords, x + dir2, y - 1))
        return;

    int dispersion = particle.physics.dispersion_rate;
    for (int d = 1; d <= dispersion; d++)
    {
        if (try_left_first)
        {
            if (try_move(chunk_coords, x, y, chunk_coords, x - d, y))
                return;
            if (try_move(chunk_coords, x, y, chunk_coords, x + d, y))
                return;
        }
        else
        {
            if (try_move(chunk_coords, x, y, chunk_coords, x + d, y))
                return;
            if (try_move(chunk_coords, x, y, chunk_coords, x - d, y))
                return;
        }
    }

    cell->particle.physics.velocity *= 0.7f;
    cell->particle.mark_updated();
}

void Falling_Sand_Simulation::apply_temperature_transfer(WorldCell *cell, const glm::ivec2 &chunk_coords, int x, int y, Chunk *current_chunk)
{
    if (!cell || cell->particle.type == Particle_Type::EMPTY)
        return;

    Particle &particle = cell->particle;

    if (particle.physics.thermal_conductivity <= 0.0f)
        return;

    // Skip almost-stable cells to avoid spending CPU on near-equilibrium noise.
    if (std::abs(particle.physics.temperature - AMBIENT_TEMPERATURE_C) < 0.5f)
        return;

    const glm::ivec2 chunk_dim = current_chunk->get_chunk_dimensions();
    float total_temp_change = 0.0f;
    int neighbor_count = 0;

    // Use 4-neighbor diffusion for cheaper and sufficiently stable heat propagation.
    const glm::ivec2 offsets[] = {
        {0, -1}, {-1, 0}, {1, 0}, {0, 1}};

    for (const auto &offset : offsets)
    {
        const int nx = x + offset.x;
        const int ny = y + offset.y;

        WorldCell *neighbor = nullptr;
        if (nx >= 0 && nx < chunk_dim.x && ny >= 0 && ny < chunk_dim.y)
        {
            neighbor = current_chunk->get_worldcell(nx, ny);
        }
        else
        {
            neighbor = get_cell_at(chunk_coords, nx, ny);
        }

        if (neighbor && neighbor->particle.type != Particle_Type::EMPTY)
        {
            float temp_diff = neighbor->particle.physics.temperature - particle.physics.temperature;
            float transfer_rate = std::min(particle.physics.thermal_conductivity,
                                           neighbor->particle.physics.thermal_conductivity);
            const float heat_capacity_factor = std::max(200.0f, particle.physics.specific_heat);
            total_temp_change += temp_diff * transfer_rate * (35.0f / heat_capacity_factor);
            neighbor_count++;
        }
    }

    if (neighbor_count > 0)
    {
        particle.physics.temperature += total_temp_change / neighbor_count;
        particle.physics.temperature = std::clamp(
            particle.physics.temperature,
            -273.15f,
            particle.physics.max_temperature);
    }
}

bool Falling_Sand_Simulation::check_state_change(Particle &particle)
{
    if (particle.type == Particle_Type::EMPTY)
        return false;

    auto preserve_runtime_state = [](Particle &target, const Particle &source)
    {
        target.physics.temperature = source.physics.temperature;
        target.physics.velocity = source.physics.velocity;
        target.physics.acceleration = source.physics.acceleration;
        target.flags.is_updated = source.flags.is_updated;
        target.flags.is_falling = source.flags.is_falling;
    };

    if (particle.physics.smoke_point > 0.0f &&
        particle.physics.temperature >= particle.physics.smoke_point &&
        particle.type != Particle_Type::SMOKE &&
        particle.type != Particle_Type::FIRE)
    {
        Particle smoked = create_smoke(false);
        preserve_runtime_state(smoked, particle);
        particle = smoked;
        return true;
    }

    if (particle.type == Particle_Type::ICE &&
        particle.physics.temperature >= particle.physics.melting_point)
    {
        Particle melted = create_water(false);
        preserve_runtime_state(melted, particle);
        particle = melted;
        return true;
    }

    if (particle.type == Particle_Type::WATER &&
        particle.physics.temperature >= particle.physics.boiling_point)
    {
        Particle vapor = create_water_vapor(false);
        preserve_runtime_state(vapor, particle);
        particle = vapor;
        return true;
    }

    if (particle.type == Particle_Type::WATER &&
        particle.physics.temperature <= particle.physics.melting_point)
    {
        Particle frozen = create_ice(false);
        preserve_runtime_state(frozen, particle);
        particle = frozen;
        return true;
    }

    if (particle.type == Particle_Type::WATER_VAPOR &&
        particle.physics.temperature < particle.physics.boiling_point)
    {
        Particle condensed = create_water(false);
        preserve_runtime_state(condensed, particle);
        particle = condensed;
        return true;
    }

    return false;
}

void Falling_Sand_Simulation::process_chunk(Chunk *chunk, const glm::ivec2 &chunk_coords)
{
    if (!chunk)
        return;

    glm::ivec2 chunk_dim = chunk->get_chunk_dimensions();
    int chunk_width = chunk_dim.x;
    int chunk_height = chunk_dim.y;

    // Process from bottom to top (so falling particles don't get processed twice)
    // Alternate left-right processing to prevent directional bias
    for (int y = chunk_height - 1; y >= 0; y--)
    {
        if (process_left_first)
        {
            for (int x = 0; x < chunk_width; x++)
            {
                WorldCell *cell = chunk->get_worldcell(x, y);
                if (!cell || cell->particle.type == Particle_Type::EMPTY)
                    continue;

                if (cell->particle.type == Particle_Type::FIRE)
                {
                    spread_fire_to_adjacent_wood(chunk_coords, x, y);
                }

                switch (cell->particle.state)
                {
                case Particle_State::SOLID:
                    update_solid(chunk_coords, x, y, cell, chunk);
                    break;
                case Particle_State::LIQUID:
                    update_liquid(chunk_coords, x, y, cell, chunk);
                    break;
                case Particle_State::GAS:
                    update_gas(chunk_coords, x, y, cell, chunk);
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            for (int x = chunk_width - 1; x >= 0; x--)
            {
                WorldCell *cell = chunk->get_worldcell(x, y);
                if (!cell || cell->particle.type == Particle_Type::EMPTY)
                    continue;

                if (cell->particle.type == Particle_Type::FIRE)
                {
                    spread_fire_to_adjacent_wood(chunk_coords, x, y);
                }

                switch (cell->particle.state)
                {
                case Particle_State::SOLID:
                    update_solid(chunk_coords, x, y, cell, chunk);
                    break;
                case Particle_State::LIQUID:
                    update_liquid(chunk_coords, x, y, cell, chunk);
                    break;
                case Particle_State::GAS:
                    update_gas(chunk_coords, x, y, cell, chunk);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void Falling_Sand_Simulation::reset_update_flags()
{
    if (!world)
        return;

    auto *active_chunks = world->get_active_chunks();

    for (const auto &chunk_coords : *active_chunks)
    {
        Chunk *chunk = get_chunk_cached(chunk_coords);
        if (!chunk)
            continue;

        auto *chunk_data = chunk->get_chunk_data();
        for (auto &cell : *chunk_data)
        {
            cell.particle.reset_update_flag();
        }
    }
}

void Falling_Sand_Simulation::update(float delta_time)
{
    if (!world)
        return;

    // Clear chunk cache for this frame
    for (auto &entry : chunk_cache)
    {
        entry.coords = {-999999, -999999};
        entry.chunk = nullptr;
    }
    cache_index = 0;

    tick_count++;

    // Alternate processing direction each tick
    process_left_first = (tick_count % 2 == 0);

    // Reset all update flags
    reset_update_flags();

    // Get active chunks and process them
    auto *active_chunks = world->get_active_chunks();

    // Process chunks - in a proper implementation you'd want to sort these
    // to process from bottom chunks to top for correct falling behavior
    std::vector<glm::ivec2> sorted_chunks(active_chunks->begin(), active_chunks->end());

    // Sort by Y descending (process bottom chunks first)
    std::sort(sorted_chunks.begin(), sorted_chunks.end(),
              [](const glm::ivec2 &a, const glm::ivec2 &b)
              {
                  return a.y > b.y; // Higher Y = lower on screen
              });

    const bool run_temperature_step = (tick_count % TEMPERATURE_UPDATE_INTERVAL == 0);

    if (run_temperature_step)
    {
        const int parity = static_cast<int>((tick_count / TEMPERATURE_UPDATE_INTERVAL) & 1ull);

        // First pass: temperature transfer and relaxation to ambient temperature.
        for (const auto &chunk_coords : sorted_chunks)
        {
            Chunk *chunk = get_chunk_cached(chunk_coords);
            if (!chunk)
                continue;

            glm::ivec2 dim = chunk->get_chunk_dimensions();
            for (int y = 0; y < dim.y; ++y)
            {
                for (int x = 0; x < dim.x; ++x)
                {
                    // Checkerboard update halves temperature workload per pass.
                    if (((x + y) & 1) != parity)
                        continue;

                    WorldCell *cell = chunk->get_worldcell(x, y);
                    if (!cell || cell->particle.type == Particle_Type::EMPTY)
                        continue;

                    apply_temperature_transfer(cell, chunk_coords, x, y, chunk);

                    Particle &particle = cell->particle;
                    particle.physics.temperature +=
                        (AMBIENT_TEMPERATURE_C - particle.physics.temperature) * AMBIENT_RELAX_RATE * delta_time;

                    particle.physics.temperature = std::clamp(
                        particle.physics.temperature,
                        -273.15f,
                        particle.physics.max_temperature);
                }
            }
        }

        // Second pass: apply phase changes caused by temperature.
        for (const auto &chunk_coords : sorted_chunks)
        {
            Chunk *chunk = get_chunk_cached(chunk_coords);
            if (!chunk)
                continue;

            bool chunk_changed = false;
            auto *chunk_data = chunk->get_chunk_data();
            for (auto &cell : *chunk_data)
            {
                if (cell.particle.type == Particle_Type::EMPTY)
                    continue;

                if (check_state_change(cell.particle))
                    chunk_changed = true;
            }

            if (chunk_changed)
                chunk->mark_dirty();
        }
    }

    // Third pass: Apply gravity to all particles
    for (const auto &chunk_coords : sorted_chunks)
    {
        Chunk *chunk = get_chunk_cached(chunk_coords);
        if (!chunk)
            continue;

        auto *chunk_data = chunk->get_chunk_data();
        for (auto &cell : *chunk_data)
        {
            if (cell.particle.type != Particle_Type::EMPTY && !cell.particle.flags.is_static)
            {
                apply_gravity(cell.particle, delta_time);
            }
        }
    }

    // Fourth pass: Process movement
    for (const auto &chunk_coords : sorted_chunks)
    {
        Chunk *chunk = get_chunk_cached(chunk_coords);
        process_chunk(chunk, chunk_coords);
    }
}
