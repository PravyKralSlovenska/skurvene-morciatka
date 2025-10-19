#include "engine/world/world.hpp"

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
    world_curr.reserve(m_rows * m_cols);
    for (int i = 0; i < m_rows; ++i)
    {
        for (int j = 0; j < m_cols; ++j)
        {
            world_curr.emplace_back(glm::vec2(j, i));
        }
    }
    world_next = world_curr;

    // world_gen = std::make_unique<Herringbone_World_Generation>(seed);
    // world_gen->load_tileset_from_image("../tilesets/template_caves_limit_connectivity.png");
    // world_gen->generate_map("../map.png", 500, 500);
    // will_figure_out_the_name_later();
}

void World::will_figure_out_the_name_later()
{
    unsigned char *pixels = world_gen->get_image_data();

    for (int y = 0; y < 500; y++)
    {
        for (int x = 0; x < 500; x += 3)
        {
            int index = (y * 500 + x);

            std::cout << "index: " << index << '\n';

            int red = static_cast<int>(pixels[index]);
            int green = static_cast<int>(pixels[index + 1]);
            int blue = static_cast<int>(pixels[index + 2]);

            if (red < 255 && green < 255 && blue < 255)
            {
                world_curr[index / 3].set_particle(create_stone());
            }
            else
            {
                world_curr[index / 3].set_particle(create_sand());
            }
        }
    }
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

        // int new_x = coords.x;
        // int new_y = coords.y;

        if (in_world_range(new_x, new_y, m_rows, m_cols))
        {
            int index = new_y * m_cols + new_x;

            if (world_curr[index].particle.type == Particle_Type::EMPTY || type == Particle_Type::EMPTY)
            {
                switch (type)
                {
                case Particle_Type::EMPTY:
                    world_curr[index].set_particle(Particle());
                    // world_curr[index].particle.~Particle();
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
            for (int x = 0; x < m_cols; x++)
            {
                update_world_decider(x, y);
            }
        }
    }
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
        // move_solid(cell);
        break;

    case Particle_State::LIQUID:
        // move_liquid(cell);
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

// nefunguje
glm::vec2 World::find_place_to_fall(WorldCell &cell)
{
    int x = cell.coords.x;
    int y = cell.coords.y;

    int velocity_x = cell.particle.velocity.velocity.x;
    int velocity_y = cell.particle.velocity.velocity.y;

    int drag;

    std::vector<Particle_State> can_fall_through;
    // switch(cell.particle.state)
    // {
    // case Particle_State::SOLID:
    //     can_fall_through = {Particle_State::GAS, Particle_State::LIQUID};
    //     break;

    // case Particle_State::LIQUID:
    //     can_fall_through = {Particle_State::GAS};
    //     break;

    // case Particle_State::GAS:
    //     break;

    // default:
    //     break;
    // }

    glm::vec2 last_valid_coords = glm::vec2(-1.0f, -1.0f);

    for (int i = 1; i <= velocity_y; i++)
    {
        int dx = x + (i * velocity_x);
        int dy = y + i;

        if (!in_world_range(dx, dy, m_rows, m_cols))
        {
            break;
        }

        WorldCell &next_cell = get_worldcell_curr(dx, dy);

        if (next_cell.particle.type == Particle_Type::EMPTY)
        {
            last_valid_coords = next_cell.coords;
        }
        // ak je miesto obsadene, tak don narazy, jak kral, po zrazke je velocity = 0;0
        else
        {
            // cell.particle.velocity.set_velocity({0.0f, 0.0f});
            break;
        }
    }

    // pridam akceleraciu
    if (cell.particle.velocity.velocity.y < cell.particle.velocity.terminal_velocity)
    {
        cell.particle.velocity.change_velocity_y(0.1);
    }

    // pridam pripocitavam nejaky ten drag
    // if (cell.particle.velocity.get_velocity_x() != 0.0f)
    // {
    //     cell.particle.velocity.change_velocity_x(-1); // nejaky drag by tam mal byt
    // }

    return last_valid_coords;
}

void World::move_solid(WorldCell &cell)
{
    int x = cell.coords.x;
    int y = cell.coords.y;

    Particle_Velocity *velocity = &cell.particle.velocity;

    WorldCell *neighbors[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    std::vector<std::pair<int, int>> way_to_neighbors = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {1, 1},
        {-1, 1},
    };

    for (int i = 0; i < 5; i++)
    {
        int dx = x + way_to_neighbors[i].first;
        int dy = y + way_to_neighbors[i].second;

        if (in_world_range(dx, dy, m_rows, m_cols))
        {
            neighbors[i] = &get_worldcell_curr(dx, dy);
        }
    }

    // dam nejaku velocity particlu
    if (neighbors[2] != nullptr && neighbors[2]->particle.type == Particle_Type::EMPTY)
    {
        velocity->change_velocity_y(1);
    }

    if (neighbors[3] != nullptr && neighbors[0] != nullptr &&
        neighbors[0]->particle.type == Particle_Type::EMPTY && neighbors[3]->particle.type == Particle_Type::EMPTY)
    {
        // velocity->change_velocity_x(-1);

        // ak miesto pod cell je obsadene, tak sa nenastavy velocity.y na 1
        // kedze sa hybem stale smerom dole, potrebujem to nastavit na 1
        // if (velocity->get_velocity_y() == 0.0f)
        // {
        //     velocity->change_velocity_y(1);
        // }
    }

    if (neighbors[1] != nullptr && neighbors[4] != nullptr &&
        neighbors[1]->particle.type == Particle_Type::EMPTY && neighbors[4]->particle.type == Particle_Type::EMPTY)
    {
        // velocity->change_velocity_x(1);

        // if (velocity->get_velocity_y() == 0.0f)
        // {
        //     velocity->change_velocity_y(1);
        // }
    }

    if (glm::vec2 coords = find_place_to_fall(cell); coords != glm::vec2(-1.0f, -1.0f))
    {
        WorldCell &target_neighbor = get_worldcell_curr(coords.x, coords.y);
        swap_particles(cell, target_neighbor);
        return;
    }

    // auto can_move = [&](WorldCell cell)
    // {
    //     Particle &particle = cell.particle;
    //     return particle.type == Particle_Type::EMPTY || particle.state == Particle_State::GAS || particle.state == Particle_State::LIQUID;
    // };

    // if (glm::vec2 coords = find_place_to_fall(cell); coords != glm::vec2(0.0f, 0.0f))
    // {
    //     auto &neighbor = get_worldcell_curr(coords.x, coords.y);
    //     swap_particles(cell, neighbor);
    //     return;
    // }

    // if (in_world_range(x, y + 1, m_rows, m_cols))
    // {
    //     WorldCell &under_cell = get_worldcell_curr(x, y + 1);
    //     if (can_move(under_cell))
    //     {
    //         swap_particles(cell, under_cell);
    //         return;
    //     }
    // }

    // bool under_left = false;
    // bool under_right = false;

    // if (in_world_range(x - 1, y + 1, m_rows, m_cols))
    // {
    //     WorldCell &under_left_cell = get_worldcell_curr(x - 1, y + 1);
    //     under_left = can_move(under_left_cell);
    // }

    // if (in_world_range(x + 1, y + 1, m_rows, m_cols))
    // {
    //     WorldCell &under_right_cell = get_worldcell_curr(x + 1, y + 1);
    //     under_right = can_move(under_right_cell);
    // }

    // if (under_left && under_right)
    // {
    //     if (rand() % 2 == 0)
    //     {
    //         WorldCell &under_left_cell = get_worldcell_curr(x - 1, y + 1);
    //         swap_particles(cell, under_left_cell);
    //     }
    //     else
    //     {
    //         WorldCell &under_right_cell = get_worldcell_curr(x + 1, y + 1);
    //         swap_particles(cell, under_right_cell);
    //     }
    // }

    // else if (under_left && !under_right)
    // {
    //     WorldCell &under_left_cell = get_worldcell_curr(x - 1, y + 1);
    //     swap_particles(cell, under_left_cell);
    // }

    // else if (!under_left && under_right)
    // {
    //     WorldCell &under_right_cell = get_worldcell_curr(x + 1, y + 1);
    //     swap_particles(cell, under_right_cell);
    // }
    // else
    // {
    //     // cell.particle.velocity.set_velocity({0, 0});
    // }
}

void World::move_liquid(WorldCell &cell)
{
    auto x = cell.coords.x;
    auto y = cell.coords.y;

    auto can_move = [&](WorldCell cell)
    {
        Particle &particle = cell.particle;
        return particle.type == Particle_Type::EMPTY || particle.state == Particle_State::GAS;
    };

    if (in_world_range(x, y + 1, m_rows, m_cols))
    {
        WorldCell &under_cell = get_worldcell_curr(x, y + 1);
        if (can_move(under_cell))
        {
            swap_particles(cell, under_cell);
            return;
        }
    }

    bool under_left = false;
    bool under_right = false;

    if (in_world_range(x - 1, y + 1, m_rows, m_cols))
    {
        WorldCell &under_left_cell = get_worldcell_curr(x - 1, y + 1);
        under_left = can_move(under_left_cell);
    }

    if (in_world_range(x + 1, y + 1, m_rows, m_cols))
    {
        WorldCell &under_right_cell = get_worldcell_curr(x + 1, y + 1);
        under_right = can_move(under_right_cell);
    }

    if (under_left && under_right)
    {
        if (rand() % 2 == 0)
        {
            WorldCell &under_left_cell = get_worldcell_curr(x - 1, y + 1);
            swap_particles(cell, under_left_cell);
        }
        else
        {
            WorldCell &under_right_cell = get_worldcell_curr(x + 1, y + 1);
            swap_particles(cell, under_right_cell);
        }
        return;
    }

    else if (under_left && !under_right)
    {
        WorldCell &under_left_cell = get_worldcell_curr(x - 1, y + 1);
        swap_particles(cell, under_left_cell);
        return;
    }

    else if (!under_left && under_right)
    {
        WorldCell &under_right_cell = get_worldcell_curr(x + 1, y + 1);
        swap_particles(cell, under_right_cell);
        return;
    }

    bool next_left = false;
    bool next_right = false;

    if (in_world_range(x - 1, y, m_rows, m_cols))
    {
        WorldCell &next_left_cell = get_worldcell_curr(x - 1, y);
        next_left = can_move(next_left_cell);
    }

    if (in_world_range(x + 1, y, m_rows, m_cols))
    {
        WorldCell &next_right_cell = get_worldcell_curr(x + 1, y);
        next_right = can_move(next_right_cell);
    }

    if (next_left && next_right)
    {
        if (rand() % 2 == 0)
        {
            WorldCell &next_left_cell = get_worldcell_curr(x - 1, y);
            swap_particles(cell, next_left_cell);
        }
        else
        {
            WorldCell &next_right_cell = get_worldcell_curr(x + 1, y);
            swap_particles(cell, next_right_cell);
        }
        return;
    }

    else if (next_left && !next_right)
    {
        WorldCell &next_left_cell = get_worldcell_curr(x - 1, y);
        swap_particles(cell, next_left_cell);
        return;
    }

    else if (!next_left && next_right)
    {
        WorldCell &next_right_cell = get_worldcell_curr(x + 1, y);
        swap_particles(cell, next_right_cell);
        return;
    }
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
