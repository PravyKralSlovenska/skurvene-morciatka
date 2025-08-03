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

    // for (int i = m_rows; i >= 0; --i)
    // {
    //     for (int j = m_cols; j >= 0; --j)
    //     {
    //         world_curr.emplace_back(glm::vec2(j, i));
    //     }
    // }
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

    bool can_move_next_left = false, can_move_next_right = false;

    if (in_world_range(x - 1, y, m_rows, m_cols))
    {
        Particle &next_left_particle = get_particle(x - 1, y);
        can_move_next_left = (next_left_particle.type == ParticleType::EMPTY);
    }

    if (in_world_range(x + 1, y, m_rows, m_cols))
    {
        Particle &next_right_particle = get_particle(x + 1, y);
        can_move_next_right = (next_right_particle.type == ParticleType::EMPTY);
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

        return;
    }

    else if (can_move_left)
    {
        Particle &target = get_particle(x + 1, y + 1);
        particle.coords = {x + 1, y + 1};
        target.coords = {x, y};
        swap_particles(particle, target);

        return;
    }

    else if (can_move_right)
    {
        Particle &target = get_particle(x - 1, y + 1);
        particle.coords = {x - 1, y + 1};
        target.coords = {x, y};
        swap_particles(particle, target);

        return;
    }

    else if (can_move_next_left && can_move_next_right)
    {
        if (rand() % 2 == 0 && can_move_next_left)
        {
            Particle &target = get_particle(x + 1, y);
            particle.coords = {x + 1, y};
            target.coords = {x, y};
            swap_particles(particle, target);
        }
        else if (can_move_next_right)
        {
            Particle &target = get_particle(x - 1, y);
            particle.coords = {x - 1, y};
            target.coords = {x, y};
            swap_particles(particle, target);
        }

        return;
    }

    else if (can_move_next_left)
    {
        Particle &target = get_particle(x + 1, y);
        particle.coords = {x + 1, y};
        target.coords = {x, y};
        swap_particles(particle, target);

        return;
    }

    else if (can_move_next_right)
    {
        Particle &target = get_particle(x - 1, y);
        particle.coords = {x - 1, y};
        target.coords = {x, y};
        swap_particles(particle, target);

        return;
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
