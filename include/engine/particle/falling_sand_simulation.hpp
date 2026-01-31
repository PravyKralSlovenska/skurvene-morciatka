#pragma once

#include <vector>
#include <random>
#include <glm/glm.hpp>

// Forward declarations
class World;
class Chunk;
struct WorldCell;
class Particle;
enum class Particle_Type;
enum class Particle_State;

/*
 * Falling_Sand_Simulation
 * - Simulacia padajuceho piesku a tekutin
 * - Spracovava pohyb castic na zaklade ich fyzikalnych vlastnosti
 * - Podporuje: gravitaciu, displacement (vymienu castic), sirenie tekutin
 */
class Falling_Sand_Simulation
{
private:
    World *world = nullptr;

    // Random number generator for non-deterministic movement choices
    std::mt19937 rng;

    // Simulation tick counter
    uint64_t tick_count = 0;

    // Physics constants
    const float GRAVITY = 50.0f;      // Gravity acceleration (cells/s²)
    const float MAX_VELOCITY = 15.0f; // Max cells per tick
    const float FRICTION = 0.95f;     // Horizontal velocity damping
    const float BOUNCE = 0.3f;        // Energy retained on collision

    // Processing order alternation (prevents bias)
    bool process_left_first = true;

private:
    // Helper: Get cell at position (handles chunk boundaries)
    WorldCell *get_cell_at(const glm::ivec2 &chunk_coords, int x, int y);
    WorldCell *get_cell_at_world_pos(const glm::ivec2 &world_cell_pos);

    // Helper: Convert local to world coordinates and vice versa
    glm::ivec2 local_to_world(const glm::ivec2 &chunk_coords, int x, int y);
    glm::ivec2 world_to_chunk(const glm::ivec2 &world_pos);
    glm::ivec2 world_to_local(const glm::ivec2 &world_pos);

    // Helper: Check if position is valid
    bool is_valid_position(const glm::ivec2 &chunk_coords, int x, int y);

    // Movement helpers
    bool try_move(const glm::ivec2 &from_chunk, int from_x, int from_y,
                  const glm::ivec2 &to_chunk, int to_x, int to_y);
    bool try_swap(WorldCell *from, WorldCell *to);

    // Particle-specific movement
    void update_solid(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell);
    void update_liquid(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell);
    void update_gas(const glm::ivec2 &chunk_coords, int x, int y, WorldCell *cell);

    // Physics updates
    void apply_gravity(Particle &particle, float delta_time);
    void apply_temperature_transfer(WorldCell *cell, const glm::ivec2 &chunk_coords, int x, int y);
    void check_state_change(Particle &particle);

    // Process single chunk
    void process_chunk(Chunk *chunk, const glm::ivec2 &chunk_coords);

public:
    Falling_Sand_Simulation();
    ~Falling_Sand_Simulation() = default;

    void set_world(World *world);

    // Main update - call once per frame
    void update(float delta_time);

    // Reset all update flags before new frame
    void reset_update_flags();

    // Get current tick
    uint64_t get_tick_count() const { return tick_count; }
};
