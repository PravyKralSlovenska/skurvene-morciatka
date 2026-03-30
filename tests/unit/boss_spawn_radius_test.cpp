#include <gtest/gtest.h>

#include <cmath>

#include "engine/player/entity.hpp"
#include "engine/player/entity_manager.hpp"

namespace
{
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

TEST(BossSpawnRadiusTest, Spawns100BossesInsideConfiguredRadiusBand)
{
    Entity_Manager manager;
    manager.set_spawn_enabled(false);

    // Keep a deterministic and test-friendly radius window.
    const float configured_min_spawn_distance = 300.0f;
    const float configured_max_spawn_distance = 460.0f;
    manager.set_spawn_distance(configured_min_spawn_distance, configured_max_spawn_distance);

    // Must mirror Entity_Manager::spawn_boss_for_completed_objective radius logic.
    const float expected_min_distance = std::max(260.0f, configured_min_spawn_distance + 40.0f);
    const float expected_max_distance = std::max(expected_min_distance + 140.0f,
                                                 configured_max_spawn_distance + 80.0f);

    const float expected_min_sq = expected_min_distance * expected_min_distance;
    const float expected_max_sq = expected_max_distance * expected_max_distance;

    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);

    auto &objective = manager.get_devushki_objective();

    for (int i = 0; i < 100; ++i)
    {
        SCOPED_TRACE(i);

        manager.remove_all_entities();
        objective.objective_complete = true;
        objective.boss_spawned = false;

        manager.update(1.0f / 60.0f);

        ASSERT_EQ(manager.get_boss_count(), 1) << "Boss failed to spawn in iteration " << i;

        Boss *boss = find_single_boss(manager);
        ASSERT_NE(boss, nullptr) << "Expected exactly one boss entity in iteration " << i;

        const float dx = static_cast<float>(boss->coords.x - player->coords.x);
        const float dy = static_cast<float>(boss->coords.y - player->coords.y);
        const float dist_sq = dx * dx + dy * dy;

        EXPECT_GE(dist_sq, expected_min_sq)
            << "Boss spawned too close in iteration " << i;
        EXPECT_LE(dist_sq, expected_max_sq)
            << "Boss spawned too far in iteration " << i;
    }
}
