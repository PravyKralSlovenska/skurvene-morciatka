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

    std::vector<glm::ivec2> offsets = calculate_offsets(chunk_radius);

    for (const auto &offset : offsets)
    {
        int chunk_pixel_width = chunk_width * Globals::PARTICLE_SIZE;
        int chunk_pixel_height = chunk_height * Globals::PARTICLE_SIZE;

        int x = floor(player->coords.x / chunk_pixel_width) + offset.x;
        int y = floor(player->coords.y / chunk_pixel_height) + offset.y;

        glm::ivec2 coords{x, y};

        add_chunk(coords);
        // world[coords]->set_state(Chunk_States::LOADED);

        // std::cout << x << ';' << y << '\n';
        active_chunks.insert(coords);
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

void World::place_particle(const glm::ivec2 position, const Particle_Type particle_type)
{
    // potrebujem nejaky offset kvoli pixelom na obrazovke
    int chunk_pixel_width = chunk_width * Globals::PARTICLE_SIZE;
    int chunk_pixel_height = chunk_height * Globals::PARTICLE_SIZE;

    glm::ivec2 chunk_pos{
        (int)floor((float)position.x / chunk_pixel_width),
        (int)floor((float)position.y / chunk_pixel_height)};

    Chunk *chunk = get_chunk(chunk_pos);
    if (!chunk)
    {
        std::cerr << "nullptr chunk: " << chunk_pos.x << ';' << chunk_pos.y << '\n';
        return;
    }

    // std::vector<glm::ivec2> offsets = calculate_offsets(2);

    // for (const auto &offset : offsets)
    {
        int pixel_offset_x = position.x - (chunk_pos.x * chunk_pixel_width);
        int pixel_offset_y = position.y - (chunk_pos.y * chunk_pixel_height);

        glm::ivec2 worldcell_pos{
            pixel_offset_x / Globals::PARTICLE_SIZE,
            pixel_offset_y / Globals::PARTICLE_SIZE};

        if (worldcell_pos.x < 0)
            worldcell_pos.x += chunk_width;
        if (worldcell_pos.y < 0)
            worldcell_pos.y += chunk_height;

        if (!in_world_range(worldcell_pos.x, worldcell_pos.y, chunk_height, chunk_width))
        {
            std::cerr << "suradnice su mimo: " << worldcell_pos.x << ';' << worldcell_pos.y << '\n';
            return;
        }

        chunk->set_worldcell(worldcell_pos, particle_type);
    }
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

Chunk *World::get_chunk(const glm::ivec2 &coords)
{
    auto it = world.find(coords);
    if (it == world.end())
    {
        return nullptr;
    }

    return it->second.get();
}

Chunk *World::get_chunk(const int x, const int y)
{
    return get_chunk(glm::ivec2{x, y});
}

Chunk *World::get_chunk(const Chunk_Coords_to_Hash something)
{
    // return world[index];
}
