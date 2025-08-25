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
    }
}

std::vector<WorldCell> &World::get_world()
{
    return world_curr;
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

    // if (in_world_range(coords.x, coords.y, m_rows, m_cols))
    // {
    //     int index = coords.y * m_cols + coords.x;

    //     if (world_curr[index].particle.type == Particle_Type::EMPTY)
    //     {
    //         switch (type)
    //         {
    //         case Particle_Type::SAND:
    //             world_curr[index].set_particle(create_sand());
    //             break;

    //         case Particle_Type::WATER:
    //             world_curr[index].set_particle(create_water());
    //             break;

    //         case Particle_Type::SMOKE:
    //             world_curr[index].set_particle(create_smoke());
    //             break;

    //         default:
    //             break;
    //         }
    //     }
    // }
}

WorldCell &World::get_worldcell(int x, int y)
{
    return world_curr[y * m_cols + x];
}

void World::swap_particles(Particle &this_particle, Particle &that_particle)
{
    std::swap(this_particle, that_particle);
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
    // case Particle_Movement::MOVE_SIDES:
    //     return glm::vec2();
    // case Particle_Movement::MOVE_SOLID:
    //     return glm::vec2();
    // case Particle_Movement::MOVE_LIQUID:
    //     return glm::vec2();
    // case Particle_Movement::MOVE_GAS:
    //     return glm::vec2();
    default:
        return glm::vec2(0, 0);
    }
}

void World::update_world_loop()
{
    for (int i = m_rows - 1; i >= 0; --i)
    {
        if (rand() % 2)
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
}

void World::update_world_decider(int x, int y)
{
    auto &cell = get_worldcell(x, y);
    auto &particle = cell.particle;
    cell.set_visited();

    if (particle.type == Particle_Type::EMPTY)
        return;
    if (particle.move == Particle_Movement::NONE)
        return;

    if ((particle.move & Particle_Movement::DOWN) && move(cell, Particle_Movement::DOWN))
        return;

    bool try_down_left_first = (rand() & 1);
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

    bool try_up_left_first = (rand() & 1);
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

    bool try_next_left_first = (rand() & 1);
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

    // std::cout << static_cast<int>(movement) << '\n';
    // std::cout << "OLD COORDS:" << x << ';' << y << '\n';
    // std::cout << "NEW COORDS:" << neighbor.coords.x << ';' << neighbor.coords.y << '\n';
    // std::cout << "--------------------------------------------\n";
    swap_particles(worldcell.particle, neighbor.particle);
    return true;
}

void World::debug_particle(int x, int y)
{
    auto &cell = get_worldcell(x, y);
    std::cout << "Particle at (" << x << ", " << y << "): "
              << "Type=" << static_cast<int>(cell.particle.type)
              << ", Move=" << static_cast<int>(cell.particle.move) << std::endl;
}

// void World::move_solid(WorldCell &worldcell)
// {
//     auto x = worldcell.coords.x;
//     auto y = worldcell.coords.y;

//     if (in_world_range(x, y + 1, m_rows, m_cols))
//     {
//         auto &under_cell = get_worldcell(x, y + 1);
//         if (under_cell.particle.type == Particle_Type::EMPTY)
//         {
//             auto &current_cell = get_worldcell(x, y);
//             swap_particles(current_cell.particle, under_cell.particle);
//             return;
//         }
//     }

//     bool can_move_left = false, can_move_right = false;

//     if (in_world_range(x + 1, y + 1, m_rows, m_cols))
//     {
//         WorldCell &under_left_cell = get_worldcell(x + 1, y + 1);
//         can_move_left = (under_left_cell.particle.type == Particle_Type::EMPTY);
//     }

//     if (in_world_range(x - 1, y + 1, m_rows, m_cols))
//     {
//         WorldCell &under_right_cell = get_worldcell(x - 1, y + 1);
//         can_move_right = (under_right_cell.particle.type == Particle_Type::EMPTY);
//     }

//     if (can_move_left && can_move_right)
//     {
//         if (rand() % 2 == 0 && can_move_left)
//         {
//             auto &current_cell = get_worldcell(x, y);
//             auto &target_cell = get_worldcell(x + 1, y + 1);
//             swap_particles(current_cell.particle, target_cell.particle);
//         }
//         else if (can_move_right)
//         {
//             auto &current_cell = get_worldcell(x, y);
//             auto &target_cell = get_worldcell(x - 1, y + 1);
//             swap_particles(current_cell.particle, target_cell.particle);
//         }
//     }

//     else if (can_move_left)
//     {
//         auto &current_cell = get_worldcell(x, y);
//         auto &target_cell = get_worldcell(x + 1, y + 1);
//         swap_particles(current_cell.particle, target_cell.particle);
//     }

//     else if (can_move_right)
//     {
//         auto &current_cell = get_worldcell(x, y);
//         auto &target_cell = get_worldcell(x - 1, y + 1);
//         swap_particles(current_cell.particle, target_cell.particle);
//     }
// }

// void World::move_gas(WorldCell &worldcell)
// {
// }

// void World::move_liquid(WorldCell &worldcell)
// {
//     auto x = worldcell.coords.x;
//     auto y = worldcell.coords.y;

//     if (in_world_range(x, y + 1, m_rows, m_cols))
//     {
//         auto &under_cell = get_worldcell(x, y + 1);
//         if (under_cell.particle.type == Particle_Type::EMPTY)
//         {
//             auto &current_cell = get_worldcell(x, y);
//             swap_particles(current_cell.particle, under_cell.particle);
//             return;
//         }
//     }

//     bool can_under_left = false, can_under_right = false;

//     if (in_world_range(x + 1, y + 1, m_rows, m_cols))
//     {
//         WorldCell &under_left_cell = get_worldcell(x + 1, y + 1);
//         can_under_left = (under_left_cell.particle.type == Particle_Type::EMPTY);
//     }

//     if (in_world_range(x - 1, y + 1, m_rows, m_cols))
//     {
//         WorldCell &under_right_cell = get_worldcell(x - 1, y + 1);
//         can_under_right = (under_right_cell.particle.type == Particle_Type::EMPTY);
//     }

//     if (can_under_left && can_under_right)
//     {
//         if (rand() % 2 == 0 && can_under_left)
//         {
//             auto &current_cell = get_worldcell(x, y);
//             auto &target_cell = get_worldcell(x + 1, y + 1);
//             swap_particles(current_cell.particle, target_cell.particle);
//         }
//         else if (can_under_right)
//         {
//             auto &current_cell = get_worldcell(x, y);
//             auto &target_cell = get_worldcell(x - 1, y + 1);
//             swap_particles(current_cell.particle, target_cell.particle);
//         }

//         return;
//     }

//     else if (can_under_left)
//     {
//         auto &current_cell = get_worldcell(x, y);
//         auto &target_cell = get_worldcell(x + 1, y + 1);
//         swap_particles(current_cell.particle, target_cell.particle);

//         return;
//     }

//     else if (can_under_right)
//     {
//         auto &current_cell = get_worldcell(x, y);
//         auto &target_cell = get_worldcell(x - 1, y + 1);
//         swap_particles(current_cell.particle, target_cell.particle);

//         return;
//     }

//     // if you cant drop down, find place where you can
//     bool move_left = false, move_right = false;
//     int lengt_left = 0, lengt_right = 0;

//     if (in_world_range(x + 1, y, m_rows, m_cols) && get_worldcell(x + 1, y).particle.state == Particle_State::NONE)
//     {
//         move_left = true;
//     }

//     if (in_world_range(x - 1, y, m_rows, m_cols) && get_worldcell(x - 1, y).particle.state == Particle_State::NONE)
//     {
//         move_right = true;
//     }

//     for (int i = 0; i < m_cols; i++)
//     {
//         if (rand() % 2 == 0)
//         {
//             if (move_left && in_world_range(x - i, y + 1, m_rows, m_cols))
//             {
//                 bool empty = (get_worldcell(x - i, y + 1).particle.state == Particle_State::NONE);
//                 if (empty)
//                 {
//                     auto &current_cell = get_worldcell(x, y);
//                     auto &target_cell = get_worldcell(x - 1, y);
//                     swap_particles(current_cell.particle, target_cell.particle);
//                     break;
//                 }
//             }
//         }
//         else
//         {
//             if (move_right && in_world_range(x + i, y + 1, m_rows, m_cols))
//             {
//                 bool empty = (get_worldcell(x + i, y + 1).particle.state == Particle_State::NONE);
//                 if (empty)
//                 {
//                     auto &current_cell = get_worldcell(x, y);
//                     auto &target_cell = get_worldcell(x + 1, y);
//                     swap_particles(current_cell.particle, target_cell.particle);
//                     break;
//                 }
//             }
//         }
//     }
// }

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
