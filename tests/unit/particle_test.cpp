#include <gtest/gtest.h>

#include "engine/particle/particle.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/particle/falling_sand_simulation.hpp"

// Basic factory tests for new particle types

TEST(ParticleFactoryTest, WoodAndFireTypes)
{
    Particle wood = create_wood(false);
    EXPECT_EQ(wood.type, Particle_Type::WOOD);
    EXPECT_EQ(wood.state, Particle_State::SOLID);
    EXPECT_TRUE(wood.flags.is_flammable);

    Particle fire = create_fire(false);
    EXPECT_EQ(fire.type, Particle_Type::FIRE);
    EXPECT_EQ(fire.state, Particle_State::GAS);
    EXPECT_GT(fire.lifetime, 0);
    EXPECT_FALSE(fire.flags.is_flammable);
    EXPECT_TRUE(fire.flags.is_on_fire);
}

// Integration test: wood adjacent to fire should ignite when simulation runs
TEST(WorldParticleTest, WoodCatchesFire)
{
    World world;
    Falling_Sand_Simulation sim;
    sim.set_world(&world);

    // create and register one chunk at 0,0 by inserting into the internal map
    auto *chunks = world.get_chunks();
    glm::ivec2 dims = world.get_chunk_dimensions();
    chunks->emplace(glm::ivec2{0, 0}, std::make_unique<Chunk>(glm::ivec2{0, 0}, dims.x, dims.y));
    Chunk *chunk = world.get_chunk(0, 0);
    ASSERT_NE(chunk, nullptr);

    // mark chunk active so simulation will process it
    world.get_active_chunks()->insert({0, 0});

    // fill chunk with empties
    // reuse dims variable from above

    std::vector<WorldCell> data;
    for (int y = 0; y < dims.y; y++)
    {
        for (int x = 0; x < dims.x; x++)
        {
            data.emplace_back(glm::ivec2(x, y));
        }
    }
    chunk->set_chunk_data(data);

    // place wood at (5,5) and fire directly below at (5,6)
    chunk->set_worldcell(5, 5, Particle_Type::WOOD, false);
    chunk->set_worldcell(5, 6, Particle_Type::FIRE, false);

    // run several simulation ticks
    for (int i = 0; i < 10; i++)
    {
        world.update(0.016f);
    }

    WorldCell *cell = chunk->get_worldcell(5, 5);
    ASSERT_NE(cell, nullptr);
    // the wood should have transformed to fire (or smoke if burned out quickly)
    EXPECT_TRUE(cell->particle.type == Particle_Type::FIRE ||
                cell->particle.type == Particle_Type::SMOKE);
}
