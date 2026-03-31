#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

#include "engine/world/world.hpp"
#include "others/GLOBALS.hpp"

TEST(StructureSpawnRulesTest, PredeterminedEntriesGeneratedForRegisteredStructures)
{
    World world;
    StructureSpawner &spawner = world.get_structure_spawner();

    const auto &blueprints = spawner.get_blueprints();
    const auto &entries = spawner.get_predetermined_entries();

    ASSERT_FALSE(blueprints.empty());
    EXPECT_EQ(entries.size(), blueprints.size());

    for (const auto &entry : entries)
    {
        EXPECT_NE(spawner.get_blueprint(entry.structure_name), nullptr);
    }
}

TEST(StructureSpawnRulesTest, DevushkiColumnTargetIsOnConfiguredCircle)
{
    World world;
    const auto &entries = world.get_structure_spawner().get_predetermined_entries();

    const auto it = std::find_if(entries.begin(), entries.end(), [](const StructureSpawner::PredeterminedEntry &entry)
                                 { return entry.structure_name == "devushki_column"; });

    ASSERT_NE(it, entries.end()) << "devushki_column must have a predetermined target";

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    EXPECT_EQ(it->target_pos.x % ps, 0);
    EXPECT_EQ(it->target_pos.y % ps, 0);

    const float radius = std::sqrt(
        static_cast<float>(it->target_pos.x * it->target_pos.x + it->target_pos.y * it->target_pos.y));

    EXPECT_NEAR(radius, static_cast<float>(5000 * ps), static_cast<float>(ps * 2));
}
