#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

#include "engine/world/world.hpp"
#include "others/GLOBALS.hpp"

TEST(StructureSpawnRulesTest, PredeterminedEntriesGeneratedForRegisteredStructures)
{
    constexpr int kNonColumnDefaultOpportunities = 10;

    World world;
    StructureSpawner &spawner = world.get_structure_spawner();

    const auto &blueprints = spawner.get_blueprints();
    const auto &entries = spawner.get_predetermined_entries();

    ASSERT_FALSE(blueprints.empty());

    size_t expected_entries = 0;
    for (const auto &blueprint : blueprints)
    {
        if (blueprint.first == "store" || blueprint.first == "devushki_store")
            continue;

        expected_entries += (blueprint.first == "devushki_column") ? 1 : kNonColumnDefaultOpportunities;
    }

    EXPECT_EQ(entries.size(), expected_entries);

    for (const auto &entry : entries)
    {
        EXPECT_NE(spawner.get_blueprint(entry.structure_name), nullptr);
    }
}

TEST(StructureSpawnRulesTest, InitialPredeterminedGenerationDoesNotPreallocateStores)
{
    World world;
    const auto &entries = world.get_structure_spawner().get_predetermined_entries();

    const int store_entry_count = static_cast<int>(std::count_if(
        entries.begin(),
        entries.end(),
        [](const StructureSpawner::PredeterminedEntry &entry)
        {
            return entry.structure_name == "store" || entry.structure_name == "devushki_store";
        }));

    EXPECT_EQ(store_entry_count, 0)
        << "Store entries should be generated lazily from explored chunk count, not at seed initialization.";
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

    EXPECT_NEAR(radius, static_cast<float>(500 * ps), static_cast<float>(ps * 2));
}
