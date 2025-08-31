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
    // std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::cout << m_cols << ';' << m_rows << '\n';

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

std::vector<WorldCell> &World::get_world_curr()
{
    return world_curr;
}

std::vector<WorldCell> &World::get_world_next()
{
    return world_next;
}

size_t World::get_index(int x, int y)
{
    return y * m_cols + x;
}

bool World::in_world_grid(int x, int y)
{
    return in_world_range(x, y, m_rows, m_cols);
}

void World::clear_world_curr()
{
    for (int i = 0; i < m_rows * m_cols; i++)
    {
        world_curr[i].particle = Particle();
        // world_curr[i].visited = false;
    }
}

void World::clear_world_next()
{
    for (int i = 0; i < m_rows * m_cols; i++)
    {
        world_next[i].particle = Particle();
        // world_next[i].visited = false;
    }
}

WorldCell &World::get_worldcell_curr(int x, int y)
{
    return world_curr[get_index(x, y)];
}

WorldCell &World::get_worldcell_next(int x, int y)
{
    return world_next[get_index(x, y)];
}

WorldCell &World::get_worldcell_curr(int index)
{
    return world_curr[index];
}

WorldCell &World::get_worldcell_next(int index)
{
    return world_next[index];
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

void World::add_particle(glm::vec2 coords, Particle_Type type, int size)
{
    // int index = get_index(coords.x, coords.y);
    // if (world_curr[index].particle.type == Particle_Type::EMPTY || type == Particle_Type::EMPTY)
    // {
    //     switch (type)
    //     {
    //     case Particle_Type::EMPTY:
    //         world_curr[index].set_particle(Particle());
    //         break;
    //     case Particle_Type::SAND:
    //         world_curr[index].set_particle(create_sand());
    //         break;
    //     case Particle_Type::WATER:
    //         world_curr[index].set_particle(create_water());
    //         break;
    //     case Particle_Type::SMOKE:
    //         world_curr[index].set_particle(create_smoke());
    //         break;
    //     case Particle_Type::STONE:
    //         world_curr[index].set_particle(create_stone());
    //         break;
    //     default:
    //         break;
    //     }
    // }

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

            if (world_curr[index].particle.type == Particle_Type::EMPTY || type == Particle_Type::EMPTY)
            {
                switch (type)
                {
                case Particle_Type::EMPTY:
                    world_curr[index].set_particle(Particle());
                    break;

                case Particle_Type::SAND:
                    world_curr[index].set_particle(create_sand());
                    break;

                case Particle_Type::SMOKE:
                    world_curr[index].set_particle(create_smoke());
                    break;

                case Particle_Type::STONE:
                    world_curr[index].set_particle(create_stone());
                    break;

                case Particle_Type::WATER:
                    world_curr[index].set_particle(create_water());
                    break;

                default:
                    break;
                }
            }
        }
    }
}

void World::swap_worlds()
{
    std::swap(world_curr, world_next);
}

void World::update_world_loop()
{
    // apply physics?

    clear_world_next();

    for (int y = m_rows - 1; y >= 0; y--)
    {
        if (rand() % 2)
        {
            for (int x = m_cols - 1; x >= 0; x--)
            {
                update_world_decider(x, y);
            }
        }
        else
        {
            for (int x = 0; x < m_cols - 1; x++)
            {
                update_world_decider(x, y);
            }
        }
    }

    swap_worlds();
    // print_world();
}

void World::update_world_decider(int x, int y)
{
    WorldCell &cell = get_worldcell_curr(x, y);

    if (cell.particle.type == Particle_Type::EMPTY || cell.particle.move == Particle_Movement::NONE) // alebo sa nevie pohnut
    {
        return;
    }

    switch (cell.particle.state)
    {
    case Particle_State::SOLID:
        move_solid(cell);
        break;

    case Particle_State::LIQUID:
        break;

    case Particle_State::GAS:
        break;

    default:
        break;
    }
}

void World::swap_particles(WorldCell &current_cell, WorldCell &target_cell)
{
    std::swap(current_cell.particle, target_cell.particle);
}

void World::move_solid(WorldCell &cell)
{
    int cell_x = cell.coords.x;
    int cell_y = cell.coords.y;

    auto move_lambda = [&](Particle_Movement movement)
    {
        glm::vec2 offset = direction_to_offset(movement);
        WorldCell &neighbor = get_worldcell_next(cell_x + offset.x, cell_y + offset.y);

        if (neighbor.particle.type == Particle_Type::EMPTY || neighbor.particle.state != Particle_State::SOLID)
        {
            swap_particles(cell, neighbor);
        }
    };

    bool can_down = in_world_grid(cell_x, cell_y + 1);

    if (can_down)
    {
        move_lambda(Particle_Movement::DOWN);
        return;
    }

    bool can_down_left = in_world_grid(cell_x - 1, cell_y + 1);
    bool can_down_right = in_world_grid(cell_x + 1, cell_y + 1);

    if (can_down_left && can_down_right)
    {
        if (rand() % 2)
        {
            move_lambda(Particle_Movement::DOWN_LEFT);
        }
        else
        {
            move_lambda(Particle_Movement::DOWN_RIGHT);
        }
    }

    else if (can_down_left && !can_down_right)
    {
        move_lambda(Particle_Movement::DOWN_LEFT);
    }

    else if (!can_down_left && can_down_right)
    {
        move_lambda(Particle_Movement::DOWN_RIGHT);
    }
}

void World::move_liquid(WorldCell &cell)
{
}

void World::debug_particle(int x, int y)
{
    auto &cell = get_worldcell_curr(x, y);
    std::cout << "Particle at (" << x << ", " << y << "): "
              << "Type=" << static_cast<int>(cell.particle.type)
              << ", Move=" << static_cast<int>(cell.particle.move) << std::endl;
}

void World::print_world()
{
    std::cout << "mapka curr:\n";
    for (int i = 0; i < m_rows; ++i)
    {
        for (int j = 0; j < m_cols; ++j)
        {
            const auto &cell = get_worldcell_curr(j, i);
            std::cout << static_cast<int>(cell.particle.type) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "---------------------------------------\n\n";

    std::cout << "mapka next:\n";
    for (int i = 0; i < m_rows; ++i)
    {
        for (int j = 0; j < m_cols; ++j)
        {
            const auto &cell = get_worldcell_next(j, i);
            std::cout << static_cast<int>(cell.particle.type) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "---------------------------------------\n\n";
}
