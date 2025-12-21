#include "engine/world/world.hpp"
#include "engine/entity.hpp"

World::World()
    : seed(1) {}

void World::set_player(Player *player)
{
    this->player = player;
}

inline int World::get_index(int x, int y)
{
    return y * width + x;
}

int World::change_width(int n)
{
    width += n;
}

int World::change_height(int n)
{
    height += n;
}

Chunk World::create_chunk(int x, int y)
{
    auto chunk = std::make_unique<Chunk>(glm::ivec2(x, y), chunk_width, chunk_height);
}

void World::add_chunk(int x, int y)
{
    add_chunk(glm::ivec2{x, y});
}

void World::add_chunk(glm::ivec2 coords)
{
    world.try_emplace(coords, std::make_unique<Chunk>(coords, chunk_width, chunk_height));
}

void World::calculate_active_chunks()
{
    active_chunks.clear();

    std::vector<glm::ivec2> offsets;

    for (int i = -chunk_radius; i <= chunk_radius; i++)
    {
        for (int j = -chunk_radius; j <= chunk_radius; j++)
        {
            float distance_from_player = std::sqrt(i * i + j * j);
            if (distance_from_player <= chunk_radius)
            {
                offsets.push_back({j, i});
            }
        }
    }

    for (const auto &offset : offsets)
    {
        int particle_size = 10;
        int chunk_pixel_width = chunk_width * particle_size;
        int chunk_pixel_height = chunk_height * particle_size;

        int x = floor(player->coords.x / chunk_pixel_width) + offset.x;
        int y = floor(player->coords.y / chunk_pixel_height) + offset.y;

        glm::ivec2 coords = {x, y};

        add_chunk(coords);
        // world[coords]->set_state(Chunk_States::LOADED);

        active_chunks.insert({x, y});
    }

    // std::cout << active_chunks.size() << '\n';
    // std::cout << world.size() << '\n';
}

void World::update()
{
    calculate_active_chunks();

    // for (const auto &active_chunk : active_chunks)
    {
        // auto chunk = *world[active_chunk];
        // std::cout << chunk.coords.x << ';' << chunk.coords.y << '\n';
        // std::cout << world.size() << "velkost" << '\n';
    }
}

void World::place_particle(glm::ivec2 position)
{
    int chunk_x = position.x / chunk_width;
    int chunk_y = position.y / chunk_height;

    // Chunk *chunk = get_chunk(chunk_x, chunk_y);

    // auto chunk = world[{chunk_x, chunk_y}];

    int cell_x = position.x - chunk_x;
    int cell_y = position.y - chunk_y;

    // WorldCell *cell = chunk->get_worldcell(cell_x, cell_y);

    // cell->set_particle(create_stone());
}

std::pair<int, int> World::get_chunk_dimensions()
{
    return std::pair<int, int>(chunk_width, chunk_height);
}

std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash> *World::get_chunks()
{
    return &world;
}

int World::get_chunks_size()
{
    return world.size();
}

std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> *World::get_active_chunks()
{
    return &active_chunks;
}

Chunk *World::get_chunk(const int x, const int y)
{
    auto it = world.find(glm::ivec2{x, y});
    return it == world.end() ? nullptr : it->second.get();
}

Chunk *World::get_chunk(const Chunk_Coords_to_Hash something)
{
    // return world[index];
}
