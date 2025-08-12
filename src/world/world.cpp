#include "engine/world/world.hpp"

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

    world_curr[10 * m_cols + 10] = create_water({50, 0});
}

void World::add_particle(glm::vec2 coords, ParticleType type)
{
    switch (type)
    {
    case ParticleType::SAND:
        world_curr[coords.y * m_cols + coords.x] = create_sand(coords);
        break;

    case ParticleType::WATER:
        world_curr[coords.y * m_cols + coords.x] = create_water(coords);
        break;

    case ParticleType::SMOKE:
        world_curr[coords.y * m_cols + coords.x] = create_smoke(coords);
        break;

    default:
        break;
    }
}

Particle &World::get_particle(int x, int y)
{
    return world_curr[y * m_cols + x];
}

void World::swap_particles(Particle &this_particle, Particle &that_particle)
{
    std::swap(this_particle, that_particle);
}

std::vector<Particle> &World::get_world()
{
    return world_curr;
}

// random generate world?

void World::update_world()
{
    for (int i = m_rows - 1; i >= 0; --i)
    {
        // for (int j = m_cols - 1; j >= 0; --j)
        for (int j = 0; j < m_cols; ++j)
        {
            auto &particle = get_particle(j, i);

            if (particle.state != ParticleState::NONE || particle.type != ParticleType::EMPTY)
            {
                switch (particle.state)
                {
                case ParticleState::SOLID:
                    move_solid(particle);
                    break;

                case ParticleState::LIQUID:
                    move_liquid(particle);
                    break;

                case ParticleState::GAS:
                    move_gas(particle);
                    break;

                default:
                    std::cerr << "nieco sa pokazilo updateworld\n";
                    break;
                }
            }
        }
    }
    // return;
    // print_world();
}

void World::move_solid(Particle &particle)
{
    auto x = particle.coords.x;
    auto y = particle.coords.y;

    if (in_world_range(x, y + 1, m_rows, m_cols))
    {
        auto &under_particle = get_particle(x, y + 1);
        if (under_particle.type == ParticleType::EMPTY)
        {
            particle.coords.y = y + 1;
            under_particle.coords.y = y;

            swap_particles(particle, under_particle);
            return;
        }
    }

    bool can_move_left = false, can_move_right = false;

    if (in_world_range(x + 1, y + 1, m_rows, m_cols))
    {
        Particle &under_left = get_particle(x + 1, y + 1);
        can_move_left = (under_left.type == ParticleType::EMPTY);
    }

    if (in_world_range(x - 1, y + 1, m_rows, m_cols))
    {
        Particle &under_right = get_particle(x - 1, y + 1);
        can_move_right = (under_right.type == ParticleType::EMPTY);
    }

    if (can_move_left && can_move_right)
    {
        if (rand() % 2 == 0 && can_move_left)
        {
            Particle &target = get_particle(x + 1, y + 1);
            particle.coords = {x + 1, y + 1};
            target.coords = {x, y};
            swap_particles(particle, target);
        }
        else if (can_move_right)
        {
            Particle &target = get_particle(x - 1, y + 1);
            particle.coords = {x - 1, y + 1};
            target.coords = {x, y};
            swap_particles(particle, target);
        }
    }

    else if (can_move_left)
    {
        Particle &target = get_particle(x + 1, y + 1);
        particle.coords = {x + 1, y + 1};
        target.coords = {x, y};
        swap_particles(particle, target);
    }

    else if (can_move_right)
    {
        Particle &target = get_particle(x - 1, y + 1);
        particle.coords = {x - 1, y + 1};
        target.coords = {x, y};
        swap_particles(particle, target);
    }
}

void World::move_gas(Particle &particle)
{
}

void World::move_liquid(Particle &particle)
{
    auto x = particle.coords.x;
    auto y = particle.coords.y;

    if (in_world_range(x, y + 1, m_rows, m_cols))
    {
        auto &under_particle = get_particle(x, y + 1);
        if (under_particle.type == ParticleType::EMPTY)
        {
            particle.coords.y = y + 1;
            under_particle.coords.y = y;

            swap_particles(particle, under_particle);
            return;
        }
    }

    bool can_under_left = false, can_under_right = false;

    if (in_world_range(x + 1, y + 1, m_rows, m_cols))
    {
        Particle &under_left = get_particle(x + 1, y + 1);
        can_under_left = (under_left.type == ParticleType::EMPTY);
    }

    if (in_world_range(x - 1, y + 1, m_rows, m_cols))
    {
        Particle &under_right = get_particle(x - 1, y + 1);
        can_under_right = (under_right.type == ParticleType::EMPTY);
    }

    if (can_under_left && can_under_right)
    {
        if (rand() % 2 == 0 && can_under_left)
        {
            Particle &target = get_particle(x + 1, y + 1);
            particle.coords = {x + 1, y + 1};
            target.coords = {x, y};
            swap_particles(particle, target);
        }
        else if (can_under_right)
        {
            Particle &target = get_particle(x - 1, y + 1);
            particle.coords = {x - 1, y + 1};
            target.coords = {x, y};
            swap_particles(particle, target);
        }

        return;
    }

    else if (can_under_left)
    {
        Particle &target = get_particle(x + 1, y + 1);
        particle.coords = {x + 1, y + 1};
        target.coords = {x, y};
        swap_particles(particle, target);

        return;
    }

    else if (can_under_right)
    {
        Particle &target = get_particle(x - 1, y + 1);
        particle.coords = {x - 1, y + 1};
        target.coords = {x, y};
        swap_particles(particle, target);

        return;
    }

    // if you cant drop down, find place where you can
    bool move_left = false, move_right = false;
    int lengt_left = 0, lengt_right = 0;

    if (get_particle(x + 1, y).state == ParticleState::NONE)
    {
        move_left = true;
    }

    if (get_particle(x - 1, y).state == ParticleState::NONE)
    {
        move_right = true;
    }

    for (int i = 0; i < m_cols; i++)
    {
        if (rand() % 2 == 0)
        {
            if (move_left && in_world_range(x - 1, y + 1, m_rows, m_cols))
            {
                bool empty = (get_particle(x - i, y + 1).state == ParticleState::NONE);
                if (empty)
                {
                    Particle &target = get_particle(x - 1, y);
                    particle.coords = {x - 1, y};
                    target.coords = {x, y};
                    swap_particles(particle, target);
                }
            }
        }
        else
        {
            if (move_right && in_world_range(x + i, y + 1, m_rows, m_cols))
            {
                bool empty = (get_particle(x + i, y + 1).state == ParticleState::NONE);
                if (empty)
                {
                    Particle &target = get_particle(x + 1, y);
                    particle.coords = {x + 1, y};
                    target.coords = {x, y};
                    swap_particles(particle, target);
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
            const auto &particle = get_particle(j, i);
            std::cout << static_cast<int>(particle.type) << ' ';
        }

        std::cout << '\n';
    }
    std::cout << "---------------------------------------\n\n";
}

// liquid
// void World::find_place_to_fall(const Particle &particle)
// {
//     auto x = particle.coords.x;
//     auto y = particle.coords.y - 1;

//     for (int i = 0; i < m_cols; i++)
//     {
//         if (in_world_range(x - 1, y, m_rows, m_cols))
//         {
//             auto &target = get_particle(x - i, y);
//             bool idk = (target.state == ParticleState::NONE);
//         }

//         if (in_world_range(x + i, y, m_rows, m_cols))
//         {
//             auto &target = get_particle(x + i, y);
//             bool idk = (target.state == ParticleState::NONE);
//         }
//     }
// }