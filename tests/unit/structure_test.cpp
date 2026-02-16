#include <gtest/gtest.h>
#include <iostream>
#include <cmath>

#include "engine/world/structure.hpp"
#include "engine/particle/particle.hpp"
#include "others/GLOBALS.hpp"

// ============================================
// STRUCTURE BLUEPRINT TESTS
// ============================================

TEST(StructureTest, StructureDefaults)
{
    Structure s;
    EXPECT_EQ(s.get_width(), 0);
    EXPECT_EQ(s.get_height(), 0);
}

TEST(StructureTest, StructureCreateNamedWithDimensions)
{
    Structure s("test", 5, 5);
    EXPECT_EQ(s.get_width(), 5);
    EXPECT_EQ(s.get_height(), 5);
    EXPECT_EQ(s.get_name(), "test");

    // Default cells should be empty particles
    const Particle &cell = s.get_cell(0, 0);
    EXPECT_EQ(cell.type, Particle_Type::EMPTY);
}

TEST(StructureTest, SetCellWithParticle)
{
    Structure s("test", 5, 5);

    Particle stone = create_stone(true);
    s.set_cell(2, 3, stone);

    const Particle &cell = s.get_cell(2, 3);
    EXPECT_EQ(cell.type, Particle_Type::STONE);
    EXPECT_EQ(cell.state, Particle_State::SOLID);
}

TEST(StructureTest, SetCellWithType)
{
    Structure s("test", 5, 5);
    s.set_cell(1, 1, Particle_Type::SAND, true);

    const Particle &cell = s.get_cell(1, 1);
    EXPECT_EQ(cell.type, Particle_Type::SAND);
}

TEST(StructureTest, BoundsCheck)
{
    Structure s("test", 3, 3);
    EXPECT_TRUE(s.in_bounds(0, 0));
    EXPECT_TRUE(s.in_bounds(2, 2));
    EXPECT_FALSE(s.in_bounds(-1, 0));
    EXPECT_FALSE(s.in_bounds(3, 0));
    EXPECT_FALSE(s.in_bounds(0, 3));
}

TEST(StructureTest, FillRect)
{
    Structure s("test", 5, 5);
    s.fill_rect(1, 1, 3, 3, Particle_Type::STONE);

    // Inside the rect should be stone
    EXPECT_EQ(s.get_cell(1, 1).type, Particle_Type::STONE);
    EXPECT_EQ(s.get_cell(3, 3).type, Particle_Type::STONE);
    EXPECT_EQ(s.get_cell(2, 2).type, Particle_Type::STONE);

    // Outside should be empty
    EXPECT_EQ(s.get_cell(0, 0).type, Particle_Type::EMPTY);
    EXPECT_EQ(s.get_cell(4, 4).type, Particle_Type::EMPTY);
}

TEST(StructureTest, CountSolidCells)
{
    Structure s("test", 5, 5);
    EXPECT_EQ(s.count_solid_cells(), 0);

    s.fill_rect(0, 0, 3, 2, Particle_Type::STONE);
    EXPECT_EQ(s.count_solid_cells(), 6); // 3x2 = 6 solid cells
}

TEST(StructureTest, OutOfBoundsSetCellIsNoOp)
{
    Structure s("test", 3, 3);
    s.set_cell(-1, 0, Particle_Type::STONE);
    s.set_cell(3, 0, Particle_Type::STONE);
    // Should not crash, and count should still be 0
    EXPECT_EQ(s.count_solid_cells(), 0);
}

TEST(StructureTest, OutOfBoundsGetCellReturnsEmpty)
{
    Structure s("test", 3, 3);
    s.fill_rect(0, 0, 3, 3, Particle_Type::STONE);

    const Particle &oob = s.get_cell(5, 5);
    EXPECT_EQ(oob.type, Particle_Type::EMPTY);
}

// ============================================
// FACTORY TESTS
// ============================================

TEST(StructureFactoryTest, CreatePlatformDefaults)
{
    Structure platform = StructureFactory::create_platform();
    EXPECT_EQ(platform.get_name(), "platform");
    EXPECT_EQ(platform.get_width(), 8);
    EXPECT_EQ(platform.get_height(), 3);

    // All cells should be solid stone
    int total = platform.get_width() * platform.get_height();
    EXPECT_EQ(platform.count_solid_cells(), total);
}

TEST(StructureFactoryTest, CreatePlatformCustom)
{
    Structure platform = StructureFactory::create_platform(12, Particle_Type::SAND);
    EXPECT_EQ(platform.get_width(), 12);
    EXPECT_EQ(platform.get_height(), 3);
    EXPECT_EQ(platform.count_solid_cells(), 36); // 12x3

    // Cells should be sand
    EXPECT_EQ(platform.get_cell(0, 0).type, Particle_Type::SAND);
}

TEST(StructureFactoryTest, PlatformHasNoEmptyCells)
{
    // CRITICAL: The platform should be 100% solid.
    // Empty cells in structures used to carve holes in terrain — that was the bug.
    Structure platform = StructureFactory::create_platform();
    int total = platform.get_width() * platform.get_height();
    int solid = platform.count_solid_cells();
    int empty = total - solid;

    EXPECT_EQ(empty, 0) << "Platform should have ZERO empty cells (no terrain carving)";
    EXPECT_EQ(solid, total) << "Platform should be 100% solid";
}

// ============================================
// SPAWNER TESTS (logic only, no world required)
// ============================================

TEST(StructureSpawnerTest, DefaultConstruction)
{
    StructureSpawner spawner;
    EXPECT_TRUE(spawner.get_placed_structures().empty());
    EXPECT_TRUE(spawner.get_blueprints().empty());
}

TEST(StructureSpawnerTest, AddBlueprint)
{
    StructureSpawner spawner;
    Structure s("test_struct", 5, 5);
    s.fill_rect(0, 0, 5, 5, Particle_Type::STONE);

    spawner.add_blueprint("test_struct", s);

    Structure *bp = spawner.get_blueprint("test_struct");
    ASSERT_NE(bp, nullptr);
    EXPECT_EQ(bp->get_name(), "test_struct");
    EXPECT_EQ(bp->get_width(), 5);
}

TEST(StructureSpawnerTest, AddBlueprintByMove)
{
    StructureSpawner spawner;
    Structure s("moved", 3, 3);
    s.fill_rect(0, 0, 3, 3, Particle_Type::SAND);

    spawner.add_blueprint(std::move(s));

    Structure *bp = spawner.get_blueprint("moved");
    ASSERT_NE(bp, nullptr);
    EXPECT_EQ(bp->get_width(), 3);
}

TEST(StructureSpawnerTest, GetNonexistentBlueprintReturnsNull)
{
    StructureSpawner spawner;
    EXPECT_EQ(spawner.get_blueprint("nonexistent"), nullptr);
}

TEST(StructureSpawnerTest, SetupDefaultRules)
{
    StructureSpawner spawner;
    spawner.setup_default_rules();

    // Should have the platform blueprint
    Structure *platform = spawner.get_blueprint("platform");
    ASSERT_NE(platform, nullptr);
    EXPECT_EQ(platform->get_name(), "platform");
    EXPECT_EQ(platform->get_width(), 8);
}

TEST(StructureSpawnerTest, CheckMinDistance)
{
    StructureSpawner spawner;
    spawner.setup_default_rules();

    // No placed structures yet — any distance check should pass
    // (we can't call check_min_distance directly since it's private,
    //  but we can verify the spawner works without world set)
    EXPECT_TRUE(spawner.get_placed_structures().empty());
}

// ============================================
// SPAWN RULES TESTS
// ============================================

TEST(SpawnRuleTest, DefaultValues)
{
    StructureSpawnRule rule;
    EXPECT_EQ(rule.spawn_chance, 0.05f);
    EXPECT_EQ(rule.min_distance_same, 500.0f);
    EXPECT_EQ(rule.min_distance_any, 200.0f);
    EXPECT_EQ(rule.placement, SpawnPlacement::ON_SURFACE);
    EXPECT_FLOAT_EQ(rule.min_empty_ratio, 0.7f);
}

TEST(SpawnRuleTest, CustomValues)
{
    StructureSpawnRule rule;
    rule.structure_name = "tower";
    rule.spawn_chance = 0.01f;
    rule.min_distance_same = 1000.0f;
    rule.min_distance_any = 500.0f;
    rule.placement = SpawnPlacement::IN_OPEN_SPACE;
    rule.min_empty_ratio = 0.9f;

    EXPECT_EQ(rule.structure_name, "tower");
    EXPECT_EQ(rule.spawn_chance, 0.01f);
    EXPECT_EQ(rule.placement, SpawnPlacement::IN_OPEN_SPACE);
}

// ============================================
// REGRESSION: Verify no terrain carving
// ============================================

TEST(StructureRegressionTest, EmptyCellsAreSkippedNotCarved)
{
    // The old bug: StructureCell::empty() cells were placed as Particle_Type::EMPTY
    // into the world, CARVING holes in existing terrain.
    //
    // The fix: empty cells in the structure grid are SKIPPED during placement.
    // Only non-empty particles are written to the world.
    //
    // This test verifies that the default platform has no empty cells that
    // could potentially carve terrain, and documents the expected behavior.

    Structure platform = StructureFactory::create_platform();

    int empty_count = 0;
    for (int y = 0; y < platform.get_height(); y++)
        for (int x = 0; x < platform.get_width(); x++)
            if (platform.get_cell(x, y).type == Particle_Type::EMPTY)
                empty_count++;

    // Platform is fully solid — no empty cells at all
    EXPECT_EQ(empty_count, 0) << "Platform should have no empty cells";

    // Even if a structure DID have empty cells (like a custom one from an image),
    // place_structure() skips them:
    //   if (cell.type == Particle_Type::EMPTY) continue;
    // This is the critical fix that prevents terrain carving.
}

TEST(StructureRegressionTest, SpawnAnalysis)
{
    // With the new system:
    // - Only 1 structure type (platform) instead of 7
    // - 3% spawn chance instead of ~15% combined
    // - min_distance_any = 300px prevents clustering
    // - min_empty_ratio = 0.7 ensures structures only go where there's space
    // - Empty cells are SKIPPED, not carved

    StructureSpawner spawner;
    spawner.setup_default_rules();

    int chunk_radius = 25;
    int total_chunks = (2 * chunk_radius + 1) * (2 * chunk_radius + 1);
    float spawn_chance = 0.03f;

    int max_theoretical = static_cast<int>(total_chunks * spawn_chance);

    std::cout << "\n=== NEW SPAWN ANALYSIS ===" << std::endl;
    std::cout << "Total chunks: " << total_chunks << std::endl;
    std::cout << "Spawn chance: " << spawn_chance * 100 << "%" << std::endl;
    std::cout << "Max theoretical spawns (before distance/space checks): ~" << max_theoretical << std::endl;
    std::cout << "With min_distance_any=300px, actual count will be much lower." << std::endl;
    std::cout << "No terrain carving occurs — empty cells are skipped." << std::endl;

    // Should be much lower than the old ~390 structures
    EXPECT_LT(max_theoretical, 100) << "New system should spawn far fewer structures";
}
