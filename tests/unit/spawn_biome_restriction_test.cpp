#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <random>
#include <vector>

#include "engine/player/entity.hpp"
#include "engine/player/entity_manager.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_biomes.hpp"
#include "engine/world/world_ca_generation.hpp"
#include "engine/world/world_chunk.hpp"
#include "others/GLOBALS.hpp"

namespace
{
    Biome_Type biome_at_world_pixel(World &world, const glm::ivec2 &pixel_pos)
    {
        World_CA_Generation *world_gen = world.get_world_gen();
        if (!world_gen)
            return Biome_Type::STONE;

        const float particle_size = static_cast<float>(Globals::PARTICLE_SIZE);
        const int world_cell_x = static_cast<int>(std::floor(static_cast<float>(pixel_pos.x) / particle_size));
        const int world_cell_y = static_cast<int>(std::floor(static_cast<float>(pixel_pos.y) / particle_size));

        const glm::vec2 world_cell_coords(
            static_cast<float>(world_cell_x),
            static_cast<float>(world_cell_y));

        return world_gen->get_biome(world_cell_coords).type;
    }

    bool is_icy(World &world, const glm::ivec2 &pixel_pos)
    {
        return biome_at_world_pixel(world, pixel_pos) == Biome_Type::ICY;
    }

    bool has_non_icy_nearby(World &world, const glm::ivec2 &center, int radius_px, int step_px)
    {
        for (int y = center.y - radius_px; y <= center.y + radius_px; y += step_px)
        {
            for (int x = center.x - radius_px; x <= center.x + radius_px; x += step_px)
            {
                if (!is_icy(world, glm::ivec2(x, y)))
                    return true;
            }
        }

        return false;
    }

    bool has_solid_support_below(Entity *entity)
    {
        if (!entity)
            return false;

        const int particle_size = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
        const int left = entity->coords.x - entity->hitbox_dimensions_half.x;
        const int right = entity->coords.x + entity->hitbox_dimensions_half.x;
        const int support_y = entity->coords.y + entity->hitbox_dimensions_half.y + particle_size;

        for (int x = left; x <= right; x += particle_size)
        {
            if (entity->is_solid_at(x, support_y))
                return true;
        }

        return entity->is_solid_at(right, support_y);
    }

    void ensure_probe_chunks(World &world, Entity *entity, int extra_down_cells = 0)
    {
        ASSERT_NE(entity, nullptr);

        auto *chunks = world.get_chunks();
        auto *world_gen = world.get_world_gen();
        ASSERT_NE(chunks, nullptr);
        ASSERT_NE(world_gen, nullptr);

        const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
        const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
        const int chunk_pixel_w = chunk_dims.x * ps;
        const int chunk_pixel_h = chunk_dims.y * ps;

        const int left = entity->coords.x - entity->hitbox_dimensions_half.x;
        const int right = entity->coords.x + entity->hitbox_dimensions_half.x;
        const int top = entity->coords.y - entity->hitbox_dimensions_half.y;
        const int bottom = entity->coords.y + entity->hitbox_dimensions_half.y + extra_down_cells * ps;

        const int first_chunk_x = static_cast<int>(std::floor(static_cast<float>(left) / chunk_pixel_w));
        const int last_chunk_x = static_cast<int>(std::floor(static_cast<float>(right) / chunk_pixel_w));
        const int first_chunk_y = static_cast<int>(std::floor(static_cast<float>(top) / chunk_pixel_h));
        const int last_chunk_y = static_cast<int>(std::floor(static_cast<float>(bottom) / chunk_pixel_h));

        for (int cy = first_chunk_y; cy <= last_chunk_y; ++cy)
        {
            for (int cx = first_chunk_x; cx <= last_chunk_x; ++cx)
            {
                const glm::ivec2 chunk_pos(cx, cy);
                if (world.get_chunk(chunk_pos))
                    continue;

                auto chunk = std::make_unique<Chunk>(chunk_pos, chunk_dims.x, chunk_dims.y);
                world_gen->generate_chunk_with_biome(chunk.get());
                chunks->emplace(chunk_pos, std::move(chunk));
            }
        }
    }

    std::vector<glm::ivec2> collect_icy_candidates(World &world)
    {
        std::vector<glm::ivec2> result;

        const int step = static_cast<int>(Globals::PARTICLE_SIZE) * 20;
        for (int y = -9000; y <= 9000; y += step)
        {
            for (int x = -9000; x <= 9000; x += step)
            {
                const glm::ivec2 pos(x, y);
                if (!is_icy(world, pos))
                    continue;

                // Require at least one nearby non-icy area so test does not rely on fallback.
                if (!has_non_icy_nearby(world, pos, 2200, step))
                    continue;

                result.push_back(pos);
                if (result.size() >= 120)
                    return result;
            }
        }

        return result;
    }

    Boss *find_single_boss(Entity_Manager &manager)
    {
        auto *entities = manager.get_all_entities();
        if (!entities)
            return nullptr;

        Boss *boss = nullptr;
        for (auto &[id, entity] : *entities)
        {
            if (!entity || entity->type != Entity_Type::BOSS)
                continue;

            if (boss != nullptr)
                return nullptr;

            boss = static_cast<Boss *>(entity.get());
        }

        return boss;
    }
} // namespace

TEST(SpawnBiomeRestrictionTest, PlayerCanSpawnInIcyBiomeWhenOtherwiseValid)
{
    World world;
    Entity_Manager manager;
    manager.set_spawn_enabled(false);
    manager.set_world(&world);

    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    std::vector<glm::ivec2> icy_candidates = collect_icy_candidates(world);
    ASSERT_FALSE(icy_candidates.empty()) << "Could not find an icy biome candidate in sampled world area.";

    bool saw_icy_result = false;

    for (int i = 0; i < 20; ++i)
    {
        const glm::ivec2 forced_icy = icy_candidates[static_cast<size_t>(i) % icy_candidates.size()];
        ASSERT_TRUE(is_icy(world, forced_icy));

        player->set_position(forced_icy);
        manager.ensure_player_valid_position();

        ensure_probe_chunks(world, player, 10);
        if (is_icy(world, player->coords))
            saw_icy_result = true;

        EXPECT_FALSE(player->check_collision_at(player->coords))
            << "Player should not overlap terrain after spawn correction from ("
            << forced_icy.x << ", " << forced_icy.y << ")"
            << " and final pos (" << player->coords.x << ", " << player->coords.y << ")";

        EXPECT_TRUE(has_solid_support_below(player))
            << "Player spawned without ground support at ("
            << player->coords.x << ", " << player->coords.y << ")";
    }

    EXPECT_TRUE(saw_icy_result)
        << "Player was always moved out of icy biome despite icy spawn now being allowed.";
}

TEST(SpawnBiomeRestrictionTest, PlayerEscapesSolidOverlap)
{
    World world;
    Entity_Manager manager;
    manager.set_spawn_enabled(false);
    manager.set_world(&world);

    auto *chunks = world.get_chunks();
    auto *world_gen = world.get_world_gen();
    ASSERT_NE(chunks, nullptr);
    ASSERT_NE(world_gen, nullptr);

    const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
    const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));

    for (int cy = -2; cy <= 2; ++cy)
    {
        for (int cx = -2; cx <= 2; ++cx)
        {
            glm::ivec2 coords(cx, cy);
            if (world.get_chunk(coords))
                continue;

            auto chunk = std::make_unique<Chunk>(coords, chunk_dims.x, chunk_dims.y);
            world_gen->generate_chunk_with_biome(chunk.get());
            chunks->emplace(coords, std::move(chunk));
        }
    }

    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    const glm::ivec2 forced_pos(0, 0);
    for (int dy = -12; dy <= 12; ++dy)
    {
        for (int dx = -10; dx <= 10; ++dx)
        {
            world.place_static_particle(forced_pos + glm::ivec2(dx * ps, dy * ps), Particle_Type::STONE);
        }
    }

    player->set_position(forced_pos);
    ASSERT_TRUE(player->check_collision_at(player->coords));

    manager.ensure_player_valid_position();

    EXPECT_FALSE(player->check_collision_at(player->coords))
        << "Player should not remain embedded in solid terrain after spawn correction";
    EXPECT_TRUE(has_solid_support_below(player))
        << "Player should be relocated onto supported ground";
}

TEST(SpawnBiomeRestrictionTest, PlayerPlacementRemainsPhysicallyValidAcrossRandomOrigins)
{
    World world;
    Entity_Manager manager;
    manager.set_spawn_enabled(false);
    manager.set_world(&world);

    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    std::mt19937 rng(1337);
    std::uniform_int_distribution<int> coord_dist(-14000, 14000);

    const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));

    for (int i = 0; i < 60; ++i)
    {
        glm::ivec2 forced_pos(coord_dist(rng), coord_dist(rng));
        forced_pos.x = static_cast<int>(std::floor(static_cast<float>(forced_pos.x) / ps)) * ps;
        forced_pos.y = static_cast<int>(std::floor(static_cast<float>(forced_pos.y) / ps)) * ps;

        player->set_position(forced_pos);
        manager.ensure_player_valid_position();

        ensure_probe_chunks(world, player, 10);

        EXPECT_FALSE(player->check_collision_at(player->coords))
            << "Player ended overlapping terrain after random forced origin ("
            << forced_pos.x << ", " << forced_pos.y << ") -> ("
            << player->coords.x << ", " << player->coords.y << ")";

        EXPECT_TRUE(has_solid_support_below(player))
            << "Player ended without solid foundation after random forced origin ("
            << forced_pos.x << ", " << forced_pos.y << ") -> ("
            << player->coords.x << ", " << player->coords.y << ")";
    }
}

TEST(SpawnBiomeRestrictionTest, PlayerPhysicsCanEscapeVerticalSeamEmbedding)
{
    World world;
    Entity_Manager manager;
    manager.set_spawn_enabled(false);
    manager.set_world(&world);

    auto *chunks = world.get_chunks();
    auto *world_gen = world.get_world_gen();
    ASSERT_NE(chunks, nullptr);
    ASSERT_NE(world_gen, nullptr);

    const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
    const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));

    for (int cy = -2; cy <= 2; ++cy)
    {
        for (int cx = -2; cx <= 2; ++cx)
        {
            glm::ivec2 coords(cx, cy);
            if (world.get_chunk(coords))
                continue;

            auto chunk = std::make_unique<Chunk>(coords, chunk_dims.x, chunk_dims.y);
            world_gen->generate_chunk_with_biome(chunk.get());
            chunks->emplace(coords, std::move(chunk));
        }
    }

    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    manager.ensure_player_valid_position();
    ensure_probe_chunks(world, player, 12);

    const glm::ivec2 initial_pos = player->coords;
    const int left = initial_pos.x - player->hitbox_dimensions_half.x;
    const int right = initial_pos.x + player->hitbox_dimensions_half.x;
    const int top = initial_pos.y - player->hitbox_dimensions_half.y - (Entity::MAX_STEP_HEIGHT + 6) * ps;
    const int bottom = initial_pos.y + player->hitbox_dimensions_half.y + 4 * ps;

    // Build a tall seam that traps the player at current X and blocks simple step-up recovery.
    for (int y = top; y <= bottom; y += ps)
    {
        for (int x = left; x <= right; x += ps)
        {
            world.place_static_particle(glm::ivec2(x, y), Particle_Type::STONE);
        }
    }

    // Keep a side corridor open so recovery must probe laterally.
    const int corridor_left = right + ps;
    const int corridor_right = right + player->hitbox_dimensions.x + 2 * ps;
    const int corridor_top = initial_pos.y - player->hitbox_dimensions_half.y - 2 * ps;
    const int corridor_bottom = initial_pos.y + player->hitbox_dimensions_half.y + 2 * ps;

    for (int y = corridor_top; y <= corridor_bottom; y += ps)
    {
        for (int x = corridor_left; x <= corridor_right; x += ps)
        {
            world.place_static_particle(glm::ivec2(x, y), Particle_Type::EMPTY);
        }
    }

    ASSERT_TRUE(player->check_collision_at(player->coords));

    player->set_velocity(0.0f, 0.0f);
    player->update_physics(1.0f / 60.0f);

    EXPECT_FALSE(player->check_collision_at(player->coords))
        << "Player remained embedded after physics recovery step.";
}

TEST(SpawnBiomeRestrictionTest, CreateEnemyDevushkiBossRemainPhysicallyValid)
{
    World world;
    Entity_Manager manager;
    manager.set_spawn_enabled(false);
    manager.set_world(&world);

    std::vector<glm::ivec2> icy_candidates = collect_icy_candidates(world);
    ASSERT_FALSE(icy_candidates.empty()) << "Could not find an icy biome candidate in sampled world area.";

    bool saw_icy_enemy_spawn = false;
    bool saw_icy_devushki_spawn = false;
    bool saw_icy_boss_spawn = false;

    for (int i = 0; i < 30; ++i)
    {
        const glm::ivec2 forced_icy = icy_candidates[static_cast<size_t>(i) % icy_candidates.size()];
        ASSERT_TRUE(is_icy(world, forced_icy));

        manager.remove_all_entities();

        Enemy *enemy = manager.create_enemy(forced_icy);
        ASSERT_NE(enemy, nullptr);
        if (is_icy(world, enemy->coords))
            saw_icy_enemy_spawn = true;
        ensure_probe_chunks(world, enemy, 2);
        EXPECT_FALSE(enemy->check_collision_at(enemy->coords)) << "Enemy spawned embedded in terrain/structure.";
        EXPECT_TRUE(has_solid_support_below(enemy)) << "Enemy spawned without solid support.";

        manager.remove_all_entities();

        Devushki *devushki = manager.create_devushki(forced_icy);
        ASSERT_NE(devushki, nullptr);
        if (is_icy(world, devushki->coords))
            saw_icy_devushki_spawn = true;
        ensure_probe_chunks(world, devushki, 2);
        EXPECT_FALSE(devushki->check_collision_at(devushki->coords)) << "Devushki spawned embedded in terrain/structure.";
        EXPECT_TRUE(has_solid_support_below(devushki)) << "Devushki spawned without solid support.";

        manager.remove_all_entities();

        Boss *boss = manager.create_boss(forced_icy);
        ASSERT_NE(boss, nullptr);
        if (is_icy(world, boss->coords))
            saw_icy_boss_spawn = true;
        ensure_probe_chunks(world, boss, 2);
        EXPECT_FALSE(boss->check_collision_at(boss->coords)) << "Boss spawned embedded in terrain/structure.";
        EXPECT_TRUE(has_solid_support_below(boss)) << "Boss spawned without solid support.";
    }

    EXPECT_TRUE(saw_icy_enemy_spawn) << "Enemy was always moved out of icy biome despite icy spawn now being allowed.";
    EXPECT_TRUE(saw_icy_devushki_spawn) << "Devushki was always moved out of icy biome despite icy spawn now being allowed.";
    EXPECT_TRUE(saw_icy_boss_spawn) << "Boss was always moved out of icy biome despite icy spawn now being allowed.";
}

TEST(SpawnBiomeRestrictionTest, ObjectiveBossSpawnRemainsPhysicallyValid)
{
    World world;
    Entity_Manager manager;
    manager.set_spawn_enabled(false);
    manager.set_world(&world);

    auto &objective = manager.get_devushki_objective();

    for (int i = 0; i < 40; ++i)
    {
        SCOPED_TRACE(i);

        manager.remove_all_entities();
        objective.objective_complete = true;
        objective.boss_spawned = false;

        manager.update(1.0f / 60.0f);

        ASSERT_EQ(manager.get_boss_count(), 1) << "Boss failed to spawn in iteration " << i;

        Boss *boss = find_single_boss(manager);
        ASSERT_NE(boss, nullptr);

        ensure_probe_chunks(world, boss, 2);
        EXPECT_FALSE(boss->check_collision_at(boss->coords))
            << "Objective boss spawned embedded at ("
            << boss->coords.x << ", " << boss->coords.y << ")";
        EXPECT_TRUE(has_solid_support_below(boss))
            << "Objective boss spawned without solid support at ("
            << boss->coords.x << ", " << boss->coords.y << ")";
    }
}

TEST(SpawnBiomeRestrictionTest, EnemySpawnDistanceSetterClampsToExclusionFloor)
{
    Entity_Manager manager;

    const float exclusion_radius_px = 50.0f * Globals::PARTICLE_SIZE;

    manager.set_spawn_distance(0.0f, 25.0f);
    SpawnConfig cfg = manager.get_spawn_config();

    EXPECT_GE(cfg.min_spawn_distance, exclusion_radius_px);
    EXPECT_GE(cfg.max_spawn_distance, cfg.min_spawn_distance + 1.0f);

    manager.set_spawn_distance(exclusion_radius_px + 30.0f, exclusion_radius_px - 5.0f);
    cfg = manager.get_spawn_config();

    EXPECT_GE(cfg.min_spawn_distance, exclusion_radius_px + 30.0f);
    EXPECT_GE(cfg.max_spawn_distance, cfg.min_spawn_distance + 1.0f);
}

TEST(SpawnBiomeRestrictionTest, RandomEnemySpawnStaysOutsidePlayerExclusionRadius)
{
    World world;
    Entity_Manager manager;
    manager.set_spawn_enabled(false);
    manager.set_world(&world);

    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    manager.ensure_player_valid_position();

    const float exclusion_radius_px = 50.0f * Globals::PARTICLE_SIZE;
    const float exclusion_radius_sq = exclusion_radius_px * exclusion_radius_px;

    // Intentionally request a too-small minimum to verify clamping during active spawn attempts.
    manager.set_spawn_distance(1.0f, exclusion_radius_px + 220.0f);

    int spawned_count = 0;
    for (int attempt = 0; attempt < 140 && spawned_count < 20; ++attempt)
    {
        Enemy *enemy = manager.spawn_random_enemy("slime");
        if (!enemy)
            continue;

        const glm::ivec2 enemy_pos = enemy->coords;
        const float dx = static_cast<float>(enemy_pos.x - player->coords.x);
        const float dy = static_cast<float>(enemy_pos.y - player->coords.y);
        const float dist_sq = dx * dx + dy * dy;

        EXPECT_GE(dist_sq, exclusion_radius_sq)
            << "Enemy spawned too close to player at ("
            << enemy_pos.x << ", " << enemy_pos.y << ") vs player ("
            << player->coords.x << ", " << player->coords.y << ")";

        manager.remove_entity(enemy->get_id());
        ++spawned_count;
    }

    EXPECT_GE(spawned_count, 10) << "Too few enemies spawned during distance regression test.";
}
