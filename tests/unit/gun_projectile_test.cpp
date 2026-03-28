#include <gtest/gtest.h>

#include <cmath>

#include "engine/player/entity.hpp"
#include "engine/player/entity_manager.hpp"
#include "engine/player/wand.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "others/GLOBALS.hpp"

namespace
{
    Particle_Type get_particle_type_at(World &world, const glm::ivec2 &pixel_pos)
    {
        const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
        const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
        const int chunk_pixel_w = chunk_dims.x * particle_size;
        const int chunk_pixel_h = chunk_dims.y * particle_size;

        const glm::ivec2 chunk_pos{
            static_cast<int>(std::floor(static_cast<float>(pixel_pos.x) / chunk_pixel_w)),
            static_cast<int>(std::floor(static_cast<float>(pixel_pos.y) / chunk_pixel_h))};

        Chunk *chunk = world.get_chunk(chunk_pos);
        if (!chunk)
            return Particle_Type::EMPTY;

        int local_x = (pixel_pos.x - chunk_pos.x * chunk_pixel_w) / particle_size;
        int local_y = (pixel_pos.y - chunk_pos.y * chunk_pixel_h) / particle_size;

        if (local_x < 0)
            local_x += chunk_dims.x;
        if (local_y < 0)
            local_y += chunk_dims.y;

        WorldCell *cell = chunk->get_worldcell(local_x, local_y);
        if (!cell)
            return Particle_Type::EMPTY;

        return cell->particle.type;
    }

    bool has_particle_in_area(World &world, const glm::ivec2 &center, int radius_px, Particle_Type type)
    {
        const int step = static_cast<int>(Globals::PARTICLE_SIZE);
        for (int y = center.y - radius_px; y <= center.y + radius_px; y += step)
        {
            for (int x = center.x - radius_px; x <= center.x + radius_px; x += step)
            {
                if (get_particle_type_at(world, glm::ivec2{x, y}) == type)
                    return true;
            }
        }

        return false;
    }

    int max_forward_fire_distance_cells(World &world, int origin_x_px)
    {
        const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
        const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
        int max_forward_cells = 0;

        auto *chunks = world.get_chunks();
        if (!chunks)
            return 0;

        for (auto &[chunk_pos, chunk_ptr] : *chunks)
        {
            Chunk *chunk = chunk_ptr.get();
            if (!chunk)
                continue;

            for (int y = 0; y < chunk_dims.y; ++y)
            {
                for (int x = 0; x < chunk_dims.x; ++x)
                {
                    WorldCell *cell = chunk->get_worldcell(x, y);
                    if (!cell || cell->particle.type != Particle_Type::FIRE)
                        continue;

                    const int world_cell_x = chunk_pos.x * chunk_dims.x + x;
                    const int world_x_px = world_cell_x * particle_size;
                    const int forward_cells = (world_x_px - origin_x_px) / particle_size;
                    if (forward_cells > max_forward_cells)
                    {
                        max_forward_cells = forward_cells;
                    }
                }
            }
        }

        return max_forward_cells;
    }
} // namespace

TEST(GunWandTest, HotbarContainsGunInDefaultSlot)
{
    Hotbar hotbar;
    const Wand &gun = hotbar.get_selected_wand();

    EXPECT_EQ(gun.type, Wand_Type::GUN_WAND);
    EXPECT_EQ(gun.brush_size, 1);
    EXPECT_GT(gun.cooldown, 0.0f);
}

TEST(ProjectileTest, EntityManagerCreatesProjectileWithPayload)
{
    World world;
    Entity_Manager manager;
    manager.set_world(&world);

    Projectile *projectile = manager.create_projectile(glm::vec2(0.0f, 0.0f),
                                                       glm::vec2(300.0f, -50.0f),
                                                       Particle_Type::SMOKE);

    ASSERT_NE(projectile, nullptr);
    EXPECT_EQ(projectile->type, Entity_Type::PROJECTILE);
    EXPECT_EQ(projectile->get_payload_type(), Particle_Type::SMOKE);
    EXPECT_TRUE(projectile->get_is_alive());
}

TEST(ProjectileTest, ProjectileMovesAndExpiresByLifetime)
{
    Projectile projectile(glm::vec2(0.0f, 0.0f), glm::vec2(200.0f, 0.0f), Particle_Type::SAND);
    projectile.set_lifetime(0.05f);

    projectile.update(0.03f);
    EXPECT_TRUE(projectile.get_is_alive());
    EXPECT_GT(projectile.coords.x, 0);

    projectile.update(0.03f);
    EXPECT_FALSE(projectile.get_is_alive());
}

TEST(FlamethrowerTest, FireParticleTravelsAtLeastTwentyCells)
{
    World world;
    Player player("FlameTester", glm::vec2(0.0f, -500.0f));
    world.set_player(&player);
    world.update(0.016f); // Ensure nearby chunks are active.

    const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
    const glm::ivec2 origin = player.coords + glm::ivec2(4 * particle_size, 0);

    // Create a clear lane so the test measures flame kinematics, not terrain collisions.
    for (int x = origin.x - 4 * particle_size; x <= origin.x + 80 * particle_size; x += particle_size)
    {
        for (int y = origin.y - 30 * particle_size; y <= origin.y + 30 * particle_size; y += particle_size)
        {
            world.place_particle(glm::ivec2(x, y), Particle_Type::EMPTY);
        }
    }

    Particle flame = create_fire(false);
    flame.physics.velocity.x = 14.5f;
    flame.physics.velocity.y = -1.0f;
    world.place_custom_particle(origin, flame);

    EXPECT_EQ(get_particle_type_at(world, origin), Particle_Type::FIRE);

    int max_forward_cells = 0;
    for (int i = 0; i < 80; ++i)
    {
        world.update(1.0f / 60.0f);
        max_forward_cells = std::max(max_forward_cells, max_forward_fire_distance_cells(world, origin.x));
    }

    EXPECT_GE(max_forward_cells, 20);
}

TEST(ProjectileTest, ProjectileCollisionDeletesParticlesInRadiusTwo)
{
    World world;
    Player player("Tester", glm::vec2(0.0f, 0.0f));
    world.set_player(&player);
    world.update(0.016f);

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    const glm::ivec2 impact_pos(40, 0);
    const glm::ivec2 neighbor_r1 = impact_pos + glm::ivec2(ps, 0);
    const glm::ivec2 neighbor_r2 = impact_pos + glm::ivec2(2 * ps, 0);
    const glm::ivec2 outside_r3 = impact_pos + glm::ivec2(3 * ps, 0);

    for (int x = -2 * ps; x <= outside_r3.x + 2 * ps; x += ps)
    {
        for (int y = -2 * ps; y <= 2 * ps; y += ps)
        {
            world.place_particle(glm::ivec2(x, y), Particle_Type::EMPTY);
        }
    }

    world.place_static_particle(impact_pos, Particle_Type::STONE);
    world.place_static_particle(neighbor_r1, Particle_Type::STONE);
    world.place_static_particle(neighbor_r2, Particle_Type::STONE);
    world.place_static_particle(outside_r3, Particle_Type::STONE);

    Entity_Manager manager;
    manager.set_world(&world);
    Projectile *projectile = manager.create_projectile(glm::vec2(0.0f, 0.0f),
                                                       glm::vec2(800.0f, 0.0f),
                                                       Particle_Type::SMOKE);

    ASSERT_NE(projectile, nullptr);

    manager.update(0.05f);

    EXPECT_FALSE(projectile->get_is_alive());
    EXPECT_EQ(get_particle_type_at(world, impact_pos), Particle_Type::EMPTY);
    EXPECT_EQ(get_particle_type_at(world, neighbor_r1), Particle_Type::EMPTY);
    EXPECT_EQ(get_particle_type_at(world, neighbor_r2), Particle_Type::EMPTY);
    EXPECT_EQ(get_particle_type_at(world, outside_r3), Particle_Type::STONE);
}

TEST(DamageSystemTest, PlayerProjectileDamagesEnemyDevushkiAndBoss)
{
    Entity_Manager manager;

    Enemy *enemy = manager.create_enemy(glm::ivec2(100, 0));
    Devushki *devushki = manager.create_devushki(glm::ivec2(200, 0));
    Boss *boss = manager.create_boss(glm::ivec2(300, 0));

    ASSERT_NE(enemy, nullptr);
    ASSERT_NE(devushki, nullptr);
    ASSERT_NE(boss, nullptr);

    const float enemy_before = enemy->healthpoints;
    const float devushki_before = devushki->healthpoints;
    const float boss_before = boss->healthpoints;

    manager.create_projectile(glm::vec2(enemy->coords), glm::vec2(0.0f, 0.0f), Particle_Type::SMOKE, 20.0f, Entity_Type::PLAYER);
    manager.create_projectile(glm::vec2(devushki->coords), glm::vec2(0.0f, 0.0f), Particle_Type::SMOKE, 20.0f, Entity_Type::PLAYER);
    manager.create_projectile(glm::vec2(boss->coords), glm::vec2(0.0f, 0.0f), Particle_Type::SMOKE, 20.0f, Entity_Type::PLAYER);

    manager.update(0.016f);

    EXPECT_LT(enemy->healthpoints, enemy_before);
    EXPECT_LT(devushki->healthpoints, devushki_before);
    EXPECT_LT(boss->healthpoints, boss_before);
}

TEST(DamageSystemTest, EnemyOwnedProjectileDamagesPlayer)
{
    Entity_Manager manager;
    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    const float hp_before = player->healthpoints;

    manager.create_projectile(glm::vec2(player->coords), glm::vec2(0.0f, 0.0f),
                              Particle_Type::SMOKE, 15.0f, Entity_Type::ENEMY);

    manager.update(0.016f);

    EXPECT_LT(player->healthpoints, hp_before);
}

TEST(DamageSystemTest, PlayerOwnedProjectileDoesNotDamagePlayer)
{
    Entity_Manager manager;
    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    const float hp_before = player->healthpoints;

    manager.create_projectile(glm::vec2(player->coords), glm::vec2(0.0f, 0.0f),
                              Particle_Type::SMOKE, 15.0f, Entity_Type::PLAYER);

    manager.update(0.016f);

    EXPECT_EQ(player->healthpoints, hp_before);
}

TEST(DamageSystemTest, EnemyMeleeAttackDamagesPlayer)
{
    Entity_Manager manager;
    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    Enemy *enemy = manager.create_enemy(player->coords);
    ASSERT_NE(enemy, nullptr);

    enemy->set_target(player->coords);
    enemy->set_attack_range(200.0f);

    const float hp_before = player->healthpoints;

    // Enough simulated time to allow enemy AI to transition to ATTACK and hit.
    for (int i = 0; i < 120; ++i)
    {
        manager.update(1.0f / 60.0f);
    }

    EXPECT_LT(player->healthpoints, hp_before);
}

TEST(DamageSystemTest, PlayerProjectileCanKillDevushki)
{
    Entity_Manager manager;
    Devushki *devushki = manager.create_devushki(glm::ivec2(150, 0));
    ASSERT_NE(devushki, nullptr);

    const int target_id = devushki->get_id();

    // Fire enough shots with enough time spacing to pass damage i-frames.
    for (int i = 0; i < 8; ++i)
    {
        if (!manager.has_entity(target_id))
            break;

        manager.create_projectile(glm::vec2(devushki->coords), glm::vec2(0.0f, 0.0f),
                                  Particle_Type::SMOKE, 25.0f, Entity_Type::PLAYER);
        manager.update(0.2f);

        Entity *remaining = manager.get_entity(target_id);
        if (!remaining)
            break;
        devushki = static_cast<Devushki *>(remaining);
    }

    EXPECT_FALSE(manager.has_entity(target_id));
}

TEST(CoinDropTest, EnemyDeathDropsCollectableGoldAndSilver)
{
    World world;
    Entity_Manager manager;
    manager.set_world(&world);

    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);
    player->set_position(0, 0);
    world.set_player(player);
    world.update(0.016f); // Ensure nearby chunks are loaded for drops

    Enemy *enemy = manager.create_enemy(player->coords);
    ASSERT_NE(enemy, nullptr);
    enemy->set_position(player->coords);
    enemy->take_damage(10000.0f);

    manager.update(0.016f);

    EXPECT_GT(manager.get_collected_gold_coins(), 0);
    EXPECT_GT(manager.get_collected_silver_coins(), 0);
}
