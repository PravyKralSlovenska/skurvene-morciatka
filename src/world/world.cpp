#include "engine/world/world.hpp"
#include <cstdlib>
#include <ctime>

/*
 * WORLDCELL
 */
WorldCell::WorldCell(glm::vec2 coords)
    : coords(coords), particle(), visited(false) {}

void WorldCell::set_particle(Particle particle)
{
    this->particle = particle;
}

void WorldCell::set_visited()
{
    visited = !visited;
}

/*
 * WORLD
 */
World::World(int w, int h, int scale)
    : m_rows(h / scale), m_cols(w / scale), scale(scale), seed(1)
{
    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    world_curr.reserve(m_rows * m_cols);
    for (size_t i = 0; i < m_rows; ++i)
    {
        for (size_t j = 0; j < m_cols; ++j)
        {
            world_curr.emplace_back(glm::vec2(j, i));
        }
    }
    world_next = world_curr;
}

void World::clear_world()
{
    for (int i = 0; i < m_rows * m_cols; i++)
    {
        world_curr[i].particle = Particle();
        world_curr[i].visited = false;
    }
}

std::vector<WorldCell> &World::get_world()
{
    return world_curr; // Return current world for rendering
}

size_t World::get_index(int x, int y)
{
    return y * m_cols + x;
}

void World::add_particle(glm::vec2 coords, Particle_Type type, int size)
{
    std::vector<glm::vec2> offsets;
    for (int y = -size; y <= size; ++y)
    {
        for (int x = -size; x <= size; ++x)
        {
            float distance = std::sqrt(x * x + y * y);
            if (distance <= size)
            {
                offsets.push_back({x, y});
            }
        }
    }
    for (const auto &offset : offsets)
    {
        int new_x = coords.x + offset.x;
        int new_y = coords.y + offset.y;
        if (in_world_range(new_x, new_y, m_rows, m_cols))
        {
            int index = new_y * m_cols + new_x;
            if (world_curr[index].particle.type == Particle_Type::EMPTY)
            {
                switch (type)
                {
                case Particle_Type::SAND:
                    world_curr[index].set_particle(create_sand());
                    break;
                case Particle_Type::WATER:
                    world_curr[index].set_particle(create_water());
                    break;
                case Particle_Type::SMOKE:
                    world_curr[index].set_particle(create_smoke());
                    break;
                default:
                    break;
                }
            }
        }
    }
}

WorldCell &World::get_worldcell(int x, int y)
{
    return world_curr[y * m_cols + x];
}

void World::move_particle(const WorldCell& source_cell, const WorldCell& target_cell)
{
    int source_x = source_cell.coords.x;
    int source_y = source_cell.coords.y;
    int target_x = target_cell.coords.x;
    int target_y = target_cell.coords.y;
    
    // Move particle from source to target in the next buffer
    world_next[get_index(target_x, target_y)].particle = source_cell.particle;
    // Clear the source position in the next buffer
    world_next[get_index(source_x, source_y)].particle = Particle();
}

void World::swap_worlds()
{
    std::swap(world_curr, world_next);
}

glm::vec2 World::direction_to_offset(Particle_Movement direction)
{
    switch (direction)
    {
    case Particle_Movement::DOWN:
        return glm::vec2(0, 1);
    case Particle_Movement::UP:
        return glm::vec2(0, -1);
    case Particle_Movement::LEFT:
        return glm::vec2(-1, 0);
    case Particle_Movement::RIGHT:
        return glm::vec2(1, 0);
    case Particle_Movement::DOWN_LEFT:
        return glm::vec2(-1, 1);
    case Particle_Movement::DOWN_RIGHT:
        return glm::vec2(1, 1);
    case Particle_Movement::UP_LEFT:
        return glm::vec2(-1, -1);
    case Particle_Movement::UP_RIGHT:
        return glm::vec2(1, -1);
    default:
        return glm::vec2(0, 0);
    }
}

void World::update_world_loop()
{
    // Copy current state to next buffer at the start of each frame
    world_next = world_curr;
    
    // Clear visited flags
    for (int i = 0; i < m_rows * m_cols; i++)
    {
        world_curr[i].visited = false;
    }
    
    // Update from bottom to top
    for (int i = m_rows - 1; i >= 0; --i)
    {
        if (std::rand() % 2)
        {
            for (int j = 0; j <= m_cols - 1; ++j)
            {
                update_world_decider(j, i);
            }
        }
        else
        {
            for (int j = m_cols - 1; j >= 0; --j)
            {
                update_world_decider(j, i);
            }
        }
    }
    
    // Swap buffers at the end
    swap_worlds();
}

void World::update_world_decider(int x, int y)
{
    auto &cell = get_worldcell(x, y);
    auto &particle = cell.particle;
    
    // Skip if already visited this frame
    if (cell.visited)
        return;
        
    cell.set_visited();
    
    if (particle.type == Particle_Type::EMPTY)
        return;
    if (particle.move == Particle_Movement::NONE)
        return;
    
    if ((particle.move & Particle_Movement::DOWN) && move(cell, Particle_Movement::DOWN))
        return;
    
    bool try_down_left_first = (std::rand() & 1);
    if (try_down_left_first)
    {
        if ((particle.move & Particle_Movement::DOWN_LEFT) && move(cell, Particle_Movement::DOWN_LEFT))
            return;
        if ((particle.move & Particle_Movement::DOWN_RIGHT) && move(cell, Particle_Movement::DOWN_RIGHT))
            return;
    }
    else
    {
        if ((particle.move & Particle_Movement::DOWN_RIGHT) && move(cell, Particle_Movement::DOWN_RIGHT))
            return;
        if ((particle.move & Particle_Movement::DOWN_LEFT) && move(cell, Particle_Movement::DOWN_LEFT))
            return;
    }
    
    if ((particle.move & Particle_Movement::UP) && move(cell, Particle_Movement::UP))
        return;
    
    bool try_up_left_first = (std::rand() & 1);
    if (try_up_left_first)
    {
        if ((particle.move & Particle_Movement::UP_LEFT) && move(cell, Particle_Movement::UP_LEFT))
            return;
        if ((particle.move & Particle_Movement::UP_RIGHT) && move(cell, Particle_Movement::UP_RIGHT))
            return;
    }
    else
    {
        if ((particle.move & Particle_Movement::UP_RIGHT) && move(cell, Particle_Movement::UP_RIGHT))
            return;
        if ((particle.move & Particle_Movement::UP_LEFT) && move(cell, Particle_Movement::UP_LEFT))
            return;
    }
    
    bool try_next_left_first = (std::rand() & 1);
    if (try_next_left_first)
    {
        if ((particle.move & Particle_Movement::LEFT) && move(cell, Particle_Movement::LEFT))
            return;
        if ((particle.move & Particle_Movement::RIGHT) && move(cell, Particle_Movement::RIGHT))
            return;
    }
    else
    {
        if ((particle.move & Particle_Movement::RIGHT) && move(cell, Particle_Movement::RIGHT))
            return;
        if ((particle.move & Particle_Movement::LEFT) && move(cell, Particle_Movement::LEFT))
            return;
    }
}

bool World::move(WorldCell &worldcell, const Particle_Movement movement)
{
    glm::vec2 offset = direction_to_offset(movement);
    auto x = worldcell.coords.x;
    auto y = worldcell.coords.y;
    
    if (!in_world_range(x + offset.x, y + offset.y, m_rows, m_cols))
    {
        return false;
    }
    
    WorldCell &neighbor = get_worldcell(x + offset.x, y + offset.y);
    if (neighbor.particle.type != Particle_Type::EMPTY)
    {
        return false;
    }
    
    WorldCell &neighbor_next = world_next[get_index(x + offset.x, y + offset.y)];
    if (neighbor_next.particle.type != Particle_Type::EMPTY)
    {
        return false;
    }

    // Move the particle using proper double buffering
    move_particle(worldcell, neighbor);
    return true;
}

void World::debug_particle(int x, int y)
{
    auto &cell = get_worldcell(x, y);
    std::cout << "Particle at (" << x << ", " << y << "): "
              << "Type=" << static_cast<int>(cell.particle.type)
              << ", Move=" << static_cast<int>(cell.particle.move) << std::endl;
}

void World::print_world()
{
    std::cout << "mapka:\n";
    for (int i = m_rows - 1; i >= 0; --i)
    {
        for (int j = 0; j < m_cols; ++j)
        {
            const auto &cell = get_worldcell(j, i);
            std::cout << static_cast<int>(cell.particle.type) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "---------------------------------------\n\n";
}