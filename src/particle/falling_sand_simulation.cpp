#include "engine/particle/falling_sand_simulation.hpp"

#include <algorithm>
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

    Chunk *chunk = world->get_chunk(actual_chunk);
    if (!chunk)
        return nullptr;

    return chunk->get_worldcell(local_pos.x, local_pos.y);
}

WorldCell *Falling_Sand_Simulation::get_cell_at_world_pos(const glm::ivec2 &world_cell_pos)
{
    glm::ivec2 chunk_coords = world_to_chunk(world_cell_pos);
    glm::ivec2 local_pos = world_to_local(world_cell_pos);

    Chunk *chunk = world->get_chunk(chunk_coords);
    if (!chunk)
        return nullptr;

    return chunk->get_worldcell(local_pos.x, local_pos.y);
}

bool Falling_Sand_Simulation::is_valid_position(const glm::ivec2 &chunk_coords, int x, int y)
{
    return get_cell_at(chunk_coords, x, y) != nullptr;
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

        Chunk *chunk_from = world->get_chunk(actual_from_chunk);
        Chunk *chunk_to = world->get_chunk(actual_to_chunk);

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

    // Apply gravity to vertical velocity
    particle.physics.velocity.y += GRAVITY * gravity_multiplier * delta_time;

    // Apply friction to horizontal velocity
    particle.physics.velocity.x *= FRICTION;

    // Clamp to max velocity
    particle.physics.velocity.y = std::clamp(particle.physics.velocity.y, -MAX_VELOCITY, MAX_VELOCITY);
    particle.physics.velocity.x = std::clamp(particle.physics.velocity.x, -MAX_VELOCITY, MAX_VELOCITY);

    // Zero out tiny velocities to prevent jitter
    if (std::abs(particle.physics.velocity.x) < 0.1f)
        particle.physics.velocity.x = 0.0f;
}

void Falling_Sand_Simulation::update_solid(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell)
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
            WorldCell *new_cell = get_cell_at(chunk_coords, x, y);
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
            WorldCell *new_cell = get_cell_at(chunk_coords, x, y);
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

void Falling_Sand_Simulation::update_liquid(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell)
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
            WorldCell *new_cell = get_cell_at(chunk_coords, x, y);
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
            WorldCell *new_cell = get_cell_at(chunk_coords, x, y);
            if (new_cell)
            {
                new_cell->particle.physics.velocity.x += dir2 * std::abs(particle.physics.velocity.y) * 0.5f;
            }
            moved = true;
            continue;
        }

        // Can't move down anymore - convert remaining vertical velocity to horizontal spread
        WorldCell *current = get_cell_at(chunk_coords, x, y);
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
            WorldCell *new_cell = get_cell_at(chunk_coords, x, y);
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

void Falling_Sand_Simulation::update_gas(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell)
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
            // Particle dissipates
            particle = Particle(); // Reset to empty
            return;
        }
    }

    // Gases rise (try to move up)
    if (try_move(chunk_coords, x, y, chunk_coords, x, y - 1))
    {
        return;
    }

    // Try diagonal up
    bool try_left_first = (rng() % 2 == 0) ^ process_left_first;
    int dir1 = try_left_first ? -1 : 1;
    int dir2 = try_left_first ? 1 : -1;

    if (try_move(chunk_coords, x, y, chunk_coords, x + dir1, y - 1))
    {
        return;
    }

    if (try_move(chunk_coords, x, y, chunk_coords, x + dir2, y - 1))
    {
        return;
    }

    // Spread horizontally
    int dispersion = particle.physics.dispersion_rate;
    for (int d = 1; d <= dispersion; d++)
    {
        if (try_left_first)
        {
            if (try_move(chunk_coords, x, y, chunk_coords, x - d, y))
            {
                return;
            }
            if (try_move(chunk_coords, x, y, chunk_coords, x + d, y))
            {
                return;
            }
        }
        else
        {
            if (try_move(chunk_coords, x, y, chunk_coords, x + d, y))
            {
                return;
            }
            if (try_move(chunk_coords, x, y, chunk_coords, x - d, y))
            {
                return;
            }
        }
    }

    // Couldn't move - mark as updated
    cell->particle.mark_updated();
}

void Falling_Sand_Simulation::apply_temperature_transfer(WorldCell *cell, const glm::ivec2 &chunk_coords, int x, int y)
{
    if (!cell || cell->particle.type == Particle_Type::EMPTY)
        return;

    Particle &particle = cell->particle;
    float total_temp_change = 0.0f;
    int neighbor_count = 0;

    // Check all 8 neighbors
    const glm::ivec2 offsets[] = {
        {-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

    for (const auto &offset : offsets)
    {
        WorldCell *neighbor = get_cell_at(chunk_coords, x + offset.x, y + offset.y);
        if (neighbor && neighbor->particle.type != Particle_Type::EMPTY)
        {
            float temp_diff = neighbor->particle.physics.temperature - particle.physics.temperature;
            float transfer_rate = std::min(particle.physics.thermal_conductivity,
                                           neighbor->particle.physics.thermal_conductivity);
            total_temp_change += temp_diff * transfer_rate * 0.1f;
            neighbor_count++;
        }
    }

    if (neighbor_count > 0)
    {
        particle.physics.temperature += total_temp_change / neighbor_count;
    }
}

void Falling_Sand_Simulation::check_state_change(Particle &particle)
{
    if (particle.type == Particle_Type::EMPTY || particle.flags.is_static)
        return;

    // Check for melting
    if (particle.state == Particle_State::SOLID &&
        particle.physics.melting_point > 0 &&
        particle.physics.temperature >= particle.physics.melting_point)
    {
        // TODO: Implement state change (e.g., stone -> lava, ice -> water)
        // For now just track the state
    }

    // Check for boiling
    if (particle.state == Particle_State::LIQUID &&
        particle.physics.boiling_point > 0 &&
        particle.physics.temperature >= particle.physics.boiling_point)
    {
        // TODO: Implement state change (e.g., water -> steam)
    }

    // Check for freezing
    if (particle.state == Particle_State::LIQUID &&
        particle.physics.melting_point > 0 &&
        particle.physics.temperature <= particle.physics.melting_point)
    {
        // TODO: Implement state change (e.g., water -> ice)
    }
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

                switch (cell->particle.state)
                {
                case Particle_State::SOLID:
                    update_solid(chunk_coords, x, y, cell);
                    break;
                case Particle_State::LIQUID:
                    update_liquid(chunk_coords, x, y, cell);
                    break;
                case Particle_State::GAS:
                    update_gas(chunk_coords, x, y, cell);
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

                switch (cell->particle.state)
                {
                case Particle_State::SOLID:
                    update_solid(chunk_coords, x, y, cell);
                    break;
                case Particle_State::LIQUID:
                    update_liquid(chunk_coords, x, y, cell);
                    break;
                case Particle_State::GAS:
                    update_gas(chunk_coords, x, y, cell);
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
        Chunk *chunk = world->get_chunk(chunk_coords);
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

    // First pass: Apply gravity to all particles
    for (const auto &chunk_coords : sorted_chunks)
    {
        Chunk *chunk = world->get_chunk(chunk_coords);
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

    // Second pass: Process movement
    for (const auto &chunk_coords : sorted_chunks)
    {
        Chunk *chunk = world->get_chunk(chunk_coords);
        process_chunk(chunk, chunk_coords);
    }
}
