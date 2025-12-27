// #include "engine/world/world_ca_generation.hpp"
// #include "engine/world/world_chunk.hpp"
// #include "engine/world/world_cell.hpp"
// #include "engine/particle/particle.hpp"
// #include "others/utils.hpp"
// #include "others/GLOBALS.hpp"

// World_CA_Generation::World_CA_Generation(int seed, float fill_percent, int iterations)
//     : seed(seed),
//       initial_fill_percent(fill_percent),
//       iterations(iterations),
//       birth_threshold(5), // Standard CA rule: >= 5 neighbors = solid
//       death_threshold(3)  // Standard CA rule: <= 3 neighbors = empty
// {
// }

// void World_CA_Generation::generate_chunk(Chunk *chunk)
// {
//     if (!chunk)
//         return;

//     // Step 1: Random initial fill
//     random_fill(chunk);

//     // Step 2: Apply CA rules multiple times
//     // Note: We can't easily access neighboring chunks during generation
//     // So we just work with the current chunk's data
//     // This will create some edge artifacts but they'll smooth out when
//     // adjacent chunks generate
//     for (int i = 0; i < iterations; i++)
//     {
//         apply_ca_iteration(chunk, {});
//     }
// }

// void World_CA_Generation::random_fill(Chunk *chunk)
// {
//     // Create deterministic random generator based on chunk position + world seed
//     int chunk_seed = hash_coords(chunk->coords.x, chunk->coords.y, seed);
//     Random rng(chunk_seed);

//     int width = chunk->width;
//     int height = chunk->height;

//     for (int y = 0; y < height; y++)
//     {
//         for (int x = 0; x < width; x++)
//         {
//             // Random fill based on percentage
//             float random_value = rng.get_float_0_to_1();

//             if (random_value < initial_fill_percent)
//             {
//                 chunk->set_worldcell(x, y, Particle_Type::STONE);
//             }
//             else
//             {
//                 chunk->set_worldcell(x, y, Particle_Type::EMPTY);
//             }
//         }
//     }
// }

// void World_CA_Generation::apply_ca_iteration(Chunk *chunk, const std::vector<WorldCell *> &neighbors_data)
// {
//     int width = chunk->width;
//     int height = chunk->height;

//     // Store next state (don't modify in-place or it affects neighbor counts)
//     std::vector<Particle_Type> next_state(width * height, Particle_Type::EMPTY);

//     for (int y = 0; y < height; y++)
//     {
//         for (int x = 0; x < width; x++)
//         {
//             int index = y * width + x;

//             // Count solid neighbors in 3x3 grid (Moore neighborhood)
//             int solid_count = 0;

//             for (int dy = -1; dy <= 1; dy++)
//             {
//                 for (int dx = -1; dx <= 1; dx++)
//                 {
//                     // Skip center cell
//                     if (dx == 0 && dy == 0)
//                         continue;

//                     int nx = x + dx;
//                     int ny = y + dy;

//                     // Treat out-of-bounds as solid (creates walls at edges)
//                     if (nx < 0 || nx >= width || ny < 0 || ny >= height)
//                     {
//                         solid_count++;
//                         continue;
//                     }

//                     // Check if neighbor is solid
//                     WorldCell *neighbor = chunk->get_worldcell(nx, ny);
//                     if (neighbor && neighbor->particle.type != Particle_Type::EMPTY)
//                     {
//                         solid_count++;
//                     }
//                 }
//             }

//             // Apply CA rules
//             if (solid_count >= birth_threshold)
//             {
//                 next_state[index] = Particle_Type::STONE;
//             }
//             else if (solid_count <= death_threshold)
//             {
//                 next_state[index] = Particle_Type::EMPTY;
//             }
//             else
//             {
//                 // Keep current state if in between thresholds
//                 WorldCell *current = chunk->get_worldcell(x, y);
//                 next_state[index] = current->particle.type;
//             }
//         }
//     }

//     // Apply next state to chunk
//     for (int y = 0; y < height; y++)
//     {
//         for (int x = 0; x < width; x++)
//         {
//             int index = y * width + x;
//             chunk->set_worldcell(x, y, next_state[index]);
//         }
//     }
// }

// void World_CA_Generation::set_parameters(float fill_percent, int iterations, int birth, int death)
// {
//     this->initial_fill_percent = fill_percent;
//     this->iterations = iterations;
//     this->birth_threshold = birth;
//     this->death_threshold = death;
// }

// void World_CA_Generation::set_seed(int seed)
// {
//     this->seed = seed;
// }