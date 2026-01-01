// #include "engine/particle/particle_movement.hpp"
// #include "engine/world/world.hpp"
// #include "engine/world/world_chunk.hpp"
// #include "engine/world/world_cell.hpp"

// WorldCell *Particle_Movement_Engine::get_cell_at(World *world, const glm::ivec2 &chunk_coords, int x, int y)
// {
//     Chunk *chunk = world->get_chunk(chunk_coords);
//     if (!chunk)
//         return nullptr;

//     auto [chunk_width, chunk_height] = world->get_chunk_dimensions();

//     // Check boundaries
//     if (x < 0 || x >= chunk_width || y < 0 || y >= chunk_height)
//         return nullptr;

//     return chunk->get_worldcell(x, y);
// }

// bool Particle_Movement_Engine::try_move_particle(World *world, const glm::ivec2 &chunk_coords, int x, int y, const glm::ivec2 &direction)
// {
//     auto [chunk_width, chunk_height] = world->get_chunk_dimensions();

//     int new_x = x + direction.x;
//     int new_y = y + direction.y;
//     glm::ivec2 new_chunk_coords = chunk_coords;

//     // Handle chunk boundary crossing
//     if (new_x < 0)
//     {
//         new_chunk_coords.x--;
//         new_x += chunk_width;
//     }
//     else if (new_x >= chunk_width)
//     {
//         new_chunk_coords.x++;
//         new_x -= chunk_width;
//     }

//     if (new_y < 0)
//     {
//         new_chunk_coords.y--;
//         new_y += chunk_height;
//     }
//     else if (new_y >= chunk_height)
//     {
//         new_chunk_coords.y++;
//         new_y -= chunk_height;
//     }

//     WorldCell *source_cell = get_cell_at(world, chunk_coords, x, y);
//     WorldCell *target_cell = get_cell_at(world, new_chunk_coords, new_x, new_y);

//     if (!source_cell || !target_cell)
//         return false;

//     // Check if target is empty
//     if (target_cell->particle.type == Particle_Type::EMPTY)
//     {
//         // Move particle
//         std::swap(source_cell->particle, target_cell->particle);
//         return true;
//     }

//     return false;
// }

// void Particle_Movement_Engine::move_solid(World *world, const glm::ivec2 &chunk_coords, int x, int y)
// {
//     WorldCell *cell = get_cell_at(world, chunk_coords, x, y);
//     if (!cell || cell->particle.type == Particle_Type::EMPTY)
//         return;

//     // Try to move down first
//     if (try_move_particle(world, chunk_coords, x, y, {0, 1}))
//         return;

//     // Try down-left
//     if (try_move_particle(world, chunk_coords, x, y, {-1, 1}))
//         return;

//     // Try down-right
//     if (try_move_particle(world, chunk_coords, x, y, {1, 1}))
//         return;
// }

// void Particle_Movement_Engine::move_liquid(World *world, const glm::ivec2 &chunk_coords, int x, int y)
// {
//     WorldCell *cell = get_cell_at(world, chunk_coords, x, y);
//     if (!cell || cell->particle.type == Particle_Type::EMPTY)
//         return;

//     // Try to move down first
//     if (try_move_particle(world, chunk_coords, x, y, {0, 1}))
//         return;

//     // Try left and right
//     if (try_move_particle(world, chunk_coords, x, y, {-1, 0}))
//         return;

//     if (try_move_particle(world, chunk_coords, x, y, {1, 0}))
//         return;

//     // Try down-left
//     if (try_move_particle(world, chunk_coords, x, y, {-1, 1}))
//         return;

//     // Try down-right
//     if (try_move_particle(world, chunk_coords, x, y, {1, 1}))
//         return;
// }

// void Particle_Movement_Engine::move_gas(World *world, const glm::ivec2 &chunk_coords, int x, int y)
// {
//     WorldCell *cell = get_cell_at(world, chunk_coords, x, y);
//     if (!cell || cell->particle.type == Particle_Type::EMPTY)
//         return;

//     // Try to move up first
//     if (try_move_particle(world, chunk_coords, x, y, {0, -1}))
//         return;

//     // Try left and right
//     if (try_move_particle(world, chunk_coords, x, y, {-1, 0}))
//         return;

//     if (try_move_particle(world, chunk_coords, x, y, {1, 0}))
//         return;

//     // Try up-left
//     if (try_move_particle(world, chunk_coords, x, y, {-1, -1}))
//         return;

//     // Try up-right
//     if (try_move_particle(world, chunk_coords, x, y, {1, -1}))
//         return;
// }

// void Particle_Movement_Engine::move(Particle *this_particle, Particle *other_particle)
// {
//     // Placeholder for interaction logic
// }