#pragma once

#include <vector>
#include <glm/glm.hpp>

class Chunk;
class WorldCell;

/*
 * World_CA_Generation
 * - Generates cave-like structures using Cellular Automata
 * - Works with chunk-based world system
 */
class World_CA_Generation
{
private:
    int seed;
    float initial_fill_percent;
    int iterations;
    int birth_threshold; // Cells with >= this many neighbors become solid
    int death_threshold; // Cells with <= this many neighbors become empty

public:
    World_CA_Generation(int seed, float fill_percent = 0.45f, int iterations = 4);
    ~World_CA_Generation() = default;

    // Generate terrain for a chunk
    void generate_chunk(Chunk *chunk);

    // Individual CA steps
    void random_fill(Chunk *chunk);
    void apply_ca_iteration(Chunk *chunk, const std::vector<WorldCell *> &neighbors_data);

    // Configuration
    void set_parameters(float fill_percent, int iterations, int birth, int death);
    void set_seed(int seed);
};
