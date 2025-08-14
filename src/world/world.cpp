#include "engine/world/world.hpp"

/*
 * WORLDCELL
 */
WorldCell::WorldCell(glm::vec2 coords)
    : coords(coords), particle() {}

void WorldCell::set_particle(Particle particle)
{
    this->particle = particle;
}

/*
 * WORLD
 */
World::World(int w, int h, int scale)
    : m_rows(h / scale), m_cols(w / scale), scale(scale), seed(1)
{
    world_curr.reserve(m_rows * m_cols);

    for (size_t i = 0; i < m_rows; ++i)
    {
        for (size_t j = 0; j < m_cols; ++j)
        {
            world_curr.emplace_back(glm::vec2(j, i));
        }
    }
}

void World::add_particle(glm::vec2 coords, ParticleType type, int size)
{
    std::vector<glm::vec2> offsets = {
        {0, 0}, // center
        {-1, -1},
        {0, -1},
        {1, -1}, // top row
        {-1, 0},
        {1, 0}, // middle row (left and right)
        {-1, 1},
        {0, 1},
        {1, 1} // bottom row
    };

    for (const auto &offset : offsets)
    {
        int new_x = coords.x + offset.x;
        int new_y = coords.y + offset.y;

        if (in_world_range(new_x, new_y, m_rows, m_cols))
        {
            int index = new_y * m_cols + new_x;

            if (world_curr[index].particle.type == ParticleType::EMPTY)
            {
                switch (type)
                {
                case ParticleType::SAND:
                    world_curr[index].set_particle(create_sand());
                    break;

                case ParticleType::WATER:
                    world_curr[index].set_particle(create_water());
                    break;

                case ParticleType::SMOKE:
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

void World::swap_particles(Particle &this_particle, Particle &that_particle)
{
    std::swap(this_particle, that_particle);
}

void World::clear_world()
{
    for (int i = 0; i < m_rows * m_cols; i++)
    {
        world_curr[i].particle = Particle();
    }
}

std::vector<WorldCell> &World::get_world()
{
    return world_curr;
}

// random generate world?

void World::update_world()
{
    for (int i = m_rows - 1; i >= 0; --i)
    {
        if (rand() % 2)
        {
            for (int j = 0; j <= m_cols - 1; ++j)
            {
                auto &cell = get_worldcell(j, i);
                auto &particle = cell.particle;

                if (particle.state != ParticleState::NONE || particle.type != ParticleType::EMPTY)
                {
                    switch (particle.state)
                    {
                    case ParticleState::SOLID:
                        move_solid(cell);
                        break;

                    case ParticleState::LIQUID:
                        move_liquid(cell);
                        break;

                    case ParticleState::GAS:
                        move_gas(cell);
                        break;

                    default:
                        std::cerr << "nieco sa pokazilo updateworld\n";
                        break;
                    }
                }
            }
        }
        else
        {
            for (int j = m_cols - 1; j >= 0; --j)
            {
                auto &cell = get_worldcell(j, i);
                auto &particle = cell.particle;

                if (particle.state != ParticleState::NONE || particle.type != ParticleType::EMPTY)
                {
                    switch (particle.state)
                    {
                    case ParticleState::SOLID:
                        move_solid(cell);
                        break;

                    case ParticleState::LIQUID:
                        move_liquid(cell);
                        break;

                    case ParticleState::GAS:
                        move_gas(cell);
                        break;

                    default:
                        std::cerr << "nieco sa pokazilo updateworld\n";
                        break;
                    }
                }
            }
        }
    }
    // return;
    // print_world();
}

void World::move_solid(WorldCell &worldcell)
{
    auto x = worldcell.coords.x;
    auto y = worldcell.coords.y;

    if (in_world_range(x, y + 1, m_rows, m_cols))
    {
        auto &under_cell = get_worldcell(x, y + 1);
        if (under_cell.particle.type == ParticleType::EMPTY)
        {
            auto &current_cell = get_worldcell(x, y);
            swap_particles(current_cell.particle, under_cell.particle);
            return;
        }
    }

    bool can_move_left = false, can_move_right = false;

    if (in_world_range(x + 1, y + 1, m_rows, m_cols))
    {
        WorldCell &under_left_cell = get_worldcell(x + 1, y + 1);
        can_move_left = (under_left_cell.particle.type == ParticleType::EMPTY);
    }

    if (in_world_range(x - 1, y + 1, m_rows, m_cols))
    {
        WorldCell &under_right_cell = get_worldcell(x - 1, y + 1);
        can_move_right = (under_right_cell.particle.type == ParticleType::EMPTY);
    }

    if (can_move_left && can_move_right)
    {
        if (rand() % 2 == 0 && can_move_left)
        {
            auto &current_cell = get_worldcell(x, y);
            auto &target_cell = get_worldcell(x + 1, y + 1);
            swap_particles(current_cell.particle, target_cell.particle);
        }
        else if (can_move_right)
        {
            auto &current_cell = get_worldcell(x, y);
            auto &target_cell = get_worldcell(x - 1, y + 1);
            swap_particles(current_cell.particle, target_cell.particle);
        }
    }

    else if (can_move_left)
    {
        auto &current_cell = get_worldcell(x, y);
        auto &target_cell = get_worldcell(x + 1, y + 1);
        swap_particles(current_cell.particle, target_cell.particle);
    }

    else if (can_move_right)
    {
        auto &current_cell = get_worldcell(x, y);
        auto &target_cell = get_worldcell(x - 1, y + 1);
        swap_particles(current_cell.particle, target_cell.particle);
    }
}

void World::move_gas(WorldCell &worldcell)
{
}

void World::move_liquid(WorldCell &worldcell)
{
    auto x = worldcell.coords.x;
    auto y = worldcell.coords.y;

    if (in_world_range(x, y + 1, m_rows, m_cols))
    {
        auto &under_cell = get_worldcell(x, y + 1);
        if (under_cell.particle.type == ParticleType::EMPTY)
        {
            auto &current_cell = get_worldcell(x, y);
            swap_particles(current_cell.particle, under_cell.particle);
            return;
        }
    }

    bool can_under_left = false, can_under_right = false;

    if (in_world_range(x + 1, y + 1, m_rows, m_cols))
    {
        WorldCell &under_left_cell = get_worldcell(x + 1, y + 1);
        can_under_left = (under_left_cell.particle.type == ParticleType::EMPTY);
    }

    if (in_world_range(x - 1, y + 1, m_rows, m_cols))
    {
        WorldCell &under_right_cell = get_worldcell(x - 1, y + 1);
        can_under_right = (under_right_cell.particle.type == ParticleType::EMPTY);
    }

    if (can_under_left && can_under_right)
    {
        if (rand() % 2 == 0 && can_under_left)
        {
            auto &current_cell = get_worldcell(x, y);
            auto &target_cell = get_worldcell(x + 1, y + 1);
            swap_particles(current_cell.particle, target_cell.particle);
        }
        else if (can_under_right)
        {
            auto &current_cell = get_worldcell(x, y);
            auto &target_cell = get_worldcell(x - 1, y + 1);
            swap_particles(current_cell.particle, target_cell.particle);
        }

        return;
    }

    else if (can_under_left)
    {
        auto &current_cell = get_worldcell(x, y);
        auto &target_cell = get_worldcell(x + 1, y + 1);
        swap_particles(current_cell.particle, target_cell.particle);

        return;
    }

    else if (can_under_right)
    {
        auto &current_cell = get_worldcell(x, y);
        auto &target_cell = get_worldcell(x - 1, y + 1);
        swap_particles(current_cell.particle, target_cell.particle);

        return;
    }

    // if you cant drop down, find place where you can
    bool move_left = false, move_right = false;
    int lengt_left = 0, lengt_right = 0;

    if (in_world_range(x + 1, y, m_rows, m_cols) && get_worldcell(x + 1, y).particle.state == ParticleState::NONE)
    {
        move_left = true;
    }

    if (in_world_range(x - 1, y, m_rows, m_cols) && get_worldcell(x - 1, y).particle.state == ParticleState::NONE)
    {
        move_right = true;
    }

    for (int i = 0; i < m_cols; i++)
    {
        if (rand() % 2 == 0)
        {
            if (move_left && in_world_range(x - i, y + 1, m_rows, m_cols))
            {
                bool empty = (get_worldcell(x - i, y + 1).particle.state == ParticleState::NONE);
                if (empty)
                {
                    auto &current_cell = get_worldcell(x, y);
                    auto &target_cell = get_worldcell(x - 1, y);
                    swap_particles(current_cell.particle, target_cell.particle);
                    break;
                }
            }
        }
        else
        {
            if (move_right && in_world_range(x + i, y + 1, m_rows, m_cols))
            {
                bool empty = (get_worldcell(x + i, y + 1).particle.state == ParticleState::NONE);
                if (empty)
                {
                    auto &current_cell = get_worldcell(x, y);
                    auto &target_cell = get_worldcell(x + 1, y);
                    swap_particles(current_cell.particle, target_cell.particle);
                    break;
                }
            }
        }
    }
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
//             bool idk = (target.state == ParticleState::NONE);
//         }

//         if (in_world_range(x + i, y, m_rows, m_cols))
//         {
//             auto &target = get_worldcell(x + i, y);
//             bool idk = (target.state == ParticleState::NONE);
//         }
//     }
// }