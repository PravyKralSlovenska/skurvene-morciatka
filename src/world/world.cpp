#include "engine/world/world.hpp"
#include "engine/entity.hpp"

int floor_integer(int a, int b)
{
    return 1;
}

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
    Chunk chunk(glm::ivec2(x, y), chunk_width, chunk_height);
}

void World::add_chunk(int x, int y)
{
    Chunk chunk = create_chunk(x, y);
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
        int x = (player->coords.x / chunk_width) + offset.x;
        int y = (player->coords.y / chunk_height) + offset.y;

        glm::ivec2 coords = {x, y};

        world.try_emplace(coords, std::make_unique<Chunk>(coords, chunk_width, chunk_height));
        // world[coords]->set_state(Chunk_States::LOADED);

        active_chunks.insert({x, y});
    }

    // std::cout << active_chunks.size() << '\n';
    // std::cout << world.size() << '\n';

    // should set all other chunks to     
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

std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Chunk_Coords_to_Hash>* World::get_chunks()
{
    return &world;
}

void World::get_active_chunks()
{
    std::vector<Chunk*> return_chunks;
}

Chunk *World::get_chunk(const int x, const int y)
{
}

Chunk *World::get_chunk(const Chunk_Coords_to_Hash something)
{
    // return world[index];
}
