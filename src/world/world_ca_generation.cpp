#include "engine/world/world_ca_generation.hpp"

#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/particle/particle.hpp"
#include "others/utils.hpp"

void World_CA_Generation::make_noise_data(Chunk *chunk)
{
    std::vector<WorldCell> new_chunk_data;
    new_chunk_data.reserve(chunk->width * chunk->height);

    for (int y{0}; y < chunk->height; y++)
    {
        for (int x{0}; x < chunk->width; x++)
        {
            if (Random_Machine::get_int_from_range(1, 100) < NOISE_DENSITY)
            {
                new_chunk_data.emplace_back(glm::ivec2(x, y), create_stone());
            }
            else
            {
                new_chunk_data.emplace_back(glm::ivec2(x, y));
            }
        }
    }

    chunk->set_chunk_data(new_chunk_data);
}

void World_CA_Generation::iteration(Chunk *chunk)
{
    std::vector<WorldCell> new_chunk_data;
    new_chunk_data.reserve(chunk->width * chunk->height);

    auto neighbors_offsets = calculate_offsets_square(1);

    auto old_chunk_data = chunk->get_chunk_data();
    for (const auto &cell : *old_chunk_data)
    {
        int solid_neighbors = 0;

        for (const auto &offset : neighbors_offsets)
        {
            glm::ivec2 neighbor_coords = cell.coords + offset;

            if (!in_world_range(neighbor_coords.x, neighbor_coords.y, chunk->height, chunk->width))
            {
                solid_neighbors++;
                continue;
            }

            // auto neighbor = chunk->get_worldcell(neighbor_coords.x, neighbor_coords.y);
            auto neighbor2 = old_chunk_data->at(neighbor_coords.y * chunk->width + neighbor_coords.x);

            if (neighbor2.particle.type != Particle_Type::EMPTY)
            {
                solid_neighbors++;
            }
        }

        if (solid_neighbors > 4)
        {
            new_chunk_data.emplace_back(cell.coords, create_stone());
        }
        // else if (solid_neighbors <= 4)
        else
        {
            new_chunk_data.emplace_back(cell.coords);
        }
    }

    if (new_chunk_data == *old_chunk_data)
    {
        chunk->set_state(Chunk_States::FINALL_FORM);
    }

    chunk->set_chunk_data(new_chunk_data);
}