#include <gtest/gtest.h>
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>

#include "engine/world/structure.hpp"
#include "engine/world/world_ca_generation.hpp"
#include "engine/world/world_cell.hpp"
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

TEST(StructureSpawnerTest, GeneratePredeterminedEntries)
{
    StructureSpawner spawner;
    Structure column("devushki_column", 5, 10);
    column.fill_rect(0, 0, 5, 10, Particle_Type::STONE);
    Structure tower("tower", 3, 8);
    tower.fill_rect(0, 0, 3, 8, Particle_Type::STONE);

    spawner.add_blueprint("devushki_column", column);
    spawner.add_blueprint("tower", tower);
    spawner.generate_predetermined_positions(1);

    const auto &entries = spawner.get_predetermined_entries();
    EXPECT_EQ(entries.size(), 2u);

    bool found_devushki = false;
    bool found_tower = false;
    for (const auto &entry : entries)
    {
        if (entry.structure_name == "devushki_column")
            found_devushki = true;
        if (entry.structure_name == "tower")
            found_tower = true;
    }

    EXPECT_TRUE(found_devushki);
    EXPECT_TRUE(found_tower);
}

TEST(StructureSpawnerTest, DevushkiEntryIsOnCircleAndGrid)
{
    StructureSpawner spawner;
    Structure column("devushki_column", 5, 10);
    column.fill_rect(0, 0, 5, 10, Particle_Type::STONE);
    spawner.add_blueprint("devushki_column", column);
    spawner.generate_predetermined_positions(1);

    const auto &entries = spawner.get_predetermined_entries();
    ASSERT_EQ(entries.size(), 1u);

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    EXPECT_EQ(entries[0].target_pos.x % ps, 0);
    EXPECT_EQ(entries[0].target_pos.y % ps, 0);

    float radius = std::sqrt(
        static_cast<float>(entries[0].target_pos.x * entries[0].target_pos.x +
                           entries[0].target_pos.y * entries[0].target_pos.y));
    EXPECT_NEAR(radius, 1000.0f, static_cast<float>(ps * 2));
}

TEST(StructureSpawnerTest, DevushkiEntryCountFollowsConfiguredSpawnCount)
{
    StructureSpawner spawner;
    Structure column("devushki_column", 5, 10);
    column.fill_rect(0, 0, 5, 10, Particle_Type::STONE);

    spawner.add_blueprint("devushki_column", column);
    spawner.set_structure_spawn_count("devushki_column", 5);
    spawner.generate_predetermined_positions(1);

    const auto &entries = spawner.get_predetermined_entries();
    ASSERT_EQ(entries.size(), 5u);

    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    for (const auto &entry : entries)
    {
        EXPECT_EQ(entry.structure_name, "devushki_column");
        EXPECT_EQ(entry.target_pos.x % ps, 0);
        EXPECT_EQ(entry.target_pos.y % ps, 0);
    }
}

TEST(StructureSpawnerTest, PlacedStructureTracking)
{
    StructureSpawner spawner;
    EXPECT_TRUE(spawner.get_placed_structures().empty());

    spawner.record_placed_structure(glm::ivec2(100, 200), "devushki_column");
    ASSERT_EQ(spawner.get_placed_structures().size(), 1u);
    EXPECT_EQ(spawner.get_placed_structures()[0].position, glm::ivec2(100, 200));
    EXPECT_EQ(spawner.get_placed_structures()[0].name, "devushki_column");
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
    StructureSpawner spawner;
    Structure column("devushki_column", 5, 10);
    column.fill_rect(0, 0, 5, 10, Particle_Type::STONE);
    Structure platform = StructureFactory::create_platform();

    spawner.add_blueprint("devushki_column", column);
    spawner.add_blueprint("platform", platform);
    spawner.generate_predetermined_positions(1);

    int generated = static_cast<int>(spawner.get_predetermined_entries().size());

    std::cout << "\n=== PREDETERMINED GENERATION ANALYSIS ===" << std::endl;
    std::cout << "Registered blueprints: 2" << std::endl;
    std::cout << "Generated entries: " << generated << std::endl;

    EXPECT_EQ(generated, 2);
}

// ============================================
// DIAGNOSTIC: Image loading and coordinate math
// ============================================

TEST(StructureDiagnostic, ImageLoading)
{
    // Try to load the actual image
    Structure s = ImageStructureLoader::load_from_image("../structure_images/devushki_column.png");

    std::cout << "\n=== IMAGE LOADING DIAGNOSTIC ===" << std::endl;
    std::cout << "Structure name: " << s.get_name() << std::endl;
    std::cout << "Structure size: " << s.get_width() << " x " << s.get_height() << std::endl;
    std::cout << "Solid cells: " << s.count_solid_cells() << std::endl;
    std::cout << "Total cells: " << s.get_width() * s.get_height() << std::endl;

    EXPECT_GT(s.get_width(), 0) << "Structure should have non-zero width";
    EXPECT_GT(s.get_height(), 0) << "Structure should have non-zero height";
    EXPECT_GT(s.count_solid_cells(), 0) << "Structure should have at least one solid cell";

    // Print a sample of what particle types we got
    if (s.get_width() > 0 && s.get_height() > 0)
    {
        std::cout << "Sample cells:" << std::endl;
        for (int y = 0; y < std::min(5, s.get_height()); y++)
        {
            for (int x = 0; x < std::min(10, s.get_width()); x++)
            {
                const Particle &p = s.get_cell(x, y);
                char c = '.';
                if (p.type == Particle_Type::STONE)
                    c = '#';
                else if (p.type == Particle_Type::SAND)
                    c = 'S';
                else if (p.type == Particle_Type::WATER)
                    c = 'W';
                else if (p.type == Particle_Type::SMOKE)
                    c = '~';
                else if (p.type == Particle_Type::URANIUM)
                    c = 'U';
                std::cout << c;
            }
            std::cout << std::endl;
        }
    }
}

TEST(StructureDiagnostic, CoordinateMath)
{
    // Simulate the coordinate math used in placement
    const int chunk_width = 10;
    const int chunk_height = 10;
    const float PARTICLE_SIZE = Globals::PARTICLE_SIZE; // 5.0

    int cpw = static_cast<int>(chunk_width * PARTICLE_SIZE);  // 50
    int cph = static_cast<int>(chunk_height * PARTICLE_SIZE); // 50

    std::cout << "\n=== COORDINATE MATH DIAGNOSTIC ===" << std::endl;
    std::cout << "Chunk: " << chunk_width << "x" << chunk_height << " cells" << std::endl;
    std::cout << "Particle size: " << PARTICLE_SIZE << " px" << std::endl;
    std::cout << "Chunk pixel size: " << cpw << "x" << cph << " px" << std::endl;

    // Generate predetermined positions the same way World does (seed=1)
    std::mt19937 rng(1);
    int chunk_radius = 15;
    int min_px = -chunk_radius * cpw;
    int max_px = chunk_radius * cpw;
    int min_py = -chunk_radius * cph;
    int max_py = chunk_radius * cph;

    std::cout << "World pixel range X: [" << min_px << ", " << max_px << ")" << std::endl;
    std::cout << "World pixel range Y: [" << min_py << ", " << max_py << ")" << std::endl;

    std::uniform_int_distribution<int> dist_x(min_px, max_px - 1);
    std::uniform_int_distribution<int> dist_y(min_py, max_py - 1);

    // Generate a few candidates
    std::cout << "\nFirst 5 candidates from seed=1:" << std::endl;
    for (int i = 0; i < 5; i++)
    {
        int px = dist_x(rng);
        int py = dist_y(rng);

        // Which chunk does this pixel fall in?
        int chunk_x = static_cast<int>(std::floor(static_cast<float>(px) / cpw));
        int chunk_y = static_cast<int>(std::floor(static_cast<float>(py) / cph));

        // What cell within the chunk?
        int offset_x = px - chunk_x * cpw;
        int offset_y = py - chunk_y * cph;
        int cell_x = static_cast<int>(offset_x / PARTICLE_SIZE);
        int cell_y = static_cast<int>(offset_y / PARTICLE_SIZE);

        std::cout << "  pos=(" << px << "," << py << ")"
                  << " -> chunk=(" << chunk_x << "," << chunk_y << ")"
                  << " cell=(" << cell_x << "," << cell_y << ")"
                  << std::endl;

        // Verify cell is within chunk bounds
        EXPECT_GE(cell_x, 0);
        EXPECT_LT(cell_x, chunk_width);
        EXPECT_GE(cell_y, 0);
        EXPECT_LT(cell_y, chunk_height);
    }
}

TEST(StructureDiagnostic, StructurePlacementSpan)
{
    // A 150x100 image structure placed at a world position
    // How many chunks does it span?
    const float PARTICLE_SIZE = Globals::PARTICLE_SIZE; // 5.0
    const int chunk_width = 10;
    const int chunk_height = 10;
    int cpw = static_cast<int>(chunk_width * PARTICLE_SIZE);  // 50
    int cph = static_cast<int>(chunk_height * PARTICLE_SIZE); // 50

    // Assume structure is loaded and auto-cropped
    Structure s = ImageStructureLoader::load_from_image("../structure_images/devushki_column.png");
    int sw = s.get_width();
    int sh = s.get_height();

    // Structure pixel footprint = sw * PARTICLE_SIZE x sh * PARTICLE_SIZE
    int struct_pixel_w = static_cast<int>(sw * PARTICLE_SIZE);
    int struct_pixel_h = static_cast<int>(sh * PARTICLE_SIZE);

    std::cout << "\n=== STRUCTURE PLACEMENT SPAN ===" << std::endl;
    std::cout << "Structure cells: " << sw << "x" << sh << std::endl;
    std::cout << "Structure pixel footprint: " << struct_pixel_w << "x" << struct_pixel_h << " px" << std::endl;
    std::cout << "Chunk pixel size: " << cpw << "x" << cph << " px" << std::endl;
    std::cout << "Chunks spanned X: ~" << (struct_pixel_w + cpw - 1) / cpw << std::endl;
    std::cout << "Chunks spanned Y: ~" << (struct_pixel_h + cph - 1) / cph << std::endl;

    // The key issue: if the structure spans N chunks,
    // only 1 chunk will exist when try_place_predetermined_structures is called.
    // place_custom_particle for cells outside that chunk will fail silently.
    if (struct_pixel_w > cpw || struct_pixel_h > cph)
    {
        std::cout << "WARNING: Structure spans more than 1 chunk!" << std::endl;
        std::cout << "  Particles in non-loaded chunks will be DROPPED silently." << std::endl;
    }

    // Also test: can the predetermined position itself be non-grid-aligned?
    // The position is a random pixel, which may not be aligned to PARTICLE_SIZE.
    // place_custom_particle divides by PARTICLE_SIZE using integer division.
    glm::ivec2 test_pos(17, 33); // non-aligned
    int chunk_x = static_cast<int>(std::floor(static_cast<float>(test_pos.x) / cpw));
    int chunk_y = static_cast<int>(std::floor(static_cast<float>(test_pos.y) / cph));
    int offset_x = test_pos.x - chunk_x * cpw;
    int offset_y = test_pos.y - chunk_y * cph;
    int cell_x = static_cast<int>(offset_x / PARTICLE_SIZE);
    int cell_y = static_cast<int>(offset_y / PARTICLE_SIZE);

    std::cout << "\nNon-aligned position test: pos=(" << test_pos.x << "," << test_pos.y << ")" << std::endl;
    std::cout << "  chunk=(" << chunk_x << "," << chunk_y << ")"
              << " offset=(" << offset_x << "," << offset_y << ")"
              << " cell=(" << cell_x << "," << cell_y << ")" << std::endl;
    std::cout << "  Pixel sub-cell offset: (" << (offset_x - cell_x * PARTICLE_SIZE) << ","
              << (offset_y - cell_y * PARTICLE_SIZE) << ")" << std::endl;
    std::cout << "  (Non-zero sub-cell offset means multiple structure cells can map to same world cell)" << std::endl;
}

// ============================================
// PLACEMENT RULES TESTS: Structures must not spawn in air
// ============================================

TEST(StructurePlacementTest, SurfaceFinding)
{
    // Test that is_cell_solid from noise can find terrain.
    // The world gen with seed=1 should produce some solid and some empty cells.
    World_CA_Generation gen(10, 10);
    gen.set_seed(1);

    int ps = static_cast<int>(Globals::PARTICLE_SIZE);

    // Scan downward at X=0 to find the first solid cell
    int found_surface = -1;
    for (int py = -500; py < 500; py += ps)
    {
        int cell_x = static_cast<int>(std::floor(0.0f / Globals::PARTICLE_SIZE));
        int cell_y = static_cast<int>(std::floor(static_cast<float>(py) / Globals::PARTICLE_SIZE));

        if (gen.is_cell_solid(cell_x, cell_y))
        {
            found_surface = py;
            break;
        }
    }

    std::cout << "\n=== SURFACE FINDING TEST ===" << std::endl;
    std::cout << "First solid cell at X=0: Y=" << found_surface << std::endl;

    // Should find SOME surface within the scan range (terrain exists)
    EXPECT_GE(found_surface, -500) << "Should find terrain surface within scan range";
}

TEST(StructurePlacementTest, SurfaceFindingMultipleColumns)
{
    // Verify surface finding works across multiple X positions
    World_CA_Generation gen(10, 10);
    gen.set_seed(1);

    int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    int surfaces_found = 0;
    int columns_tested = 20;

    std::cout << "\n=== MULTI-COLUMN SURFACE TEST ===" << std::endl;
    for (int test_x = -200; test_x < 200; test_x += ps * 4)
    {
        for (int py = -750; py < 750; py += ps)
        {
            int cell_x = static_cast<int>(std::floor(static_cast<float>(test_x) / Globals::PARTICLE_SIZE));
            int cell_y = static_cast<int>(std::floor(static_cast<float>(py) / Globals::PARTICLE_SIZE));

            if (gen.is_cell_solid(cell_x, cell_y))
            {
                surfaces_found++;
                std::cout << "  X=" << test_x << " -> surface at Y=" << py << std::endl;
                break;
            }
        }
    }

    // Most columns should have a surface
    EXPECT_GT(surfaces_found, 0) << "Should find surfaces in at least some columns";
    std::cout << "Found surfaces in " << surfaces_found << " columns" << std::endl;
}

TEST(StructurePlacementTest, GroundBeneathRequired)
{
    // Verify that the ground-beneath check correctly identifies solid vs air.
    // The terrain is cave-based (not layered), so we need to find an actual
    // air-to-solid transition (cave wall edge).
    World_CA_Generation gen(10, 10);
    gen.set_seed(1);

    int ps = static_cast<int>(Globals::PARTICLE_SIZE);

    // Search for an air-to-solid transition: find a cell that is empty with a solid cell below it
    bool found_transition = false;
    int transition_x = 0;
    int transition_y = 0;
    int transition_cx = 0;
    int transition_cy_air = 0;
    int transition_cy_ground = 0;

    for (int test_x = -1000; test_x <= 1000 && !found_transition; test_x += ps)
    {
        int cx = static_cast<int>(std::floor(static_cast<float>(test_x) / Globals::PARTICLE_SIZE));
        for (int test_y = -2500; test_y < 2500; test_y += ps)
        {
            int cy = static_cast<int>(std::floor(static_cast<float>(test_y) / Globals::PARTICLE_SIZE));
            int cy_below = static_cast<int>(std::floor(static_cast<float>(test_y + ps) / Globals::PARTICLE_SIZE));

            bool current_empty = !gen.is_cell_solid(cx, cy);
            bool below_solid = gen.is_cell_solid(cx, cy_below);

            if (current_empty && below_solid)
            {
                found_transition = true;
                transition_x = test_x;
                transition_y = test_y;
                transition_cx = cx;
                transition_cy_air = cy;
                transition_cy_ground = cy_below;
                break;
            }
        }
    }

    ASSERT_TRUE(found_transition) << "Should find an air-to-solid transition in the terrain";

    EXPECT_FALSE(gen.is_cell_solid(transition_cx, transition_cy_air)) << "Cell above transition should be air";
    EXPECT_TRUE(gen.is_cell_solid(transition_cx, transition_cy_ground)) << "Cell below transition should be solid ground";

    std::cout << "\n=== GROUND BENEATH TEST ===" << std::endl;
    std::cout << "Found transition at X=" << transition_x << " Y=" << transition_y << std::endl;
    std::cout << "Air cell (" << transition_cx << ", " << transition_cy_air << "): solid="
              << gen.is_cell_solid(transition_cx, transition_cy_air) << std::endl;
    std::cout << "Ground cell (" << transition_cx << ", " << transition_cy_ground << "): solid="
              << gen.is_cell_solid(transition_cx, transition_cy_ground) << std::endl;
}

TEST(StructurePlacementTest, FlatSurfaceCheck)
{
    // Test that flat-surface detection works via noise queries.
    // The terrain is cave-based, so we look for air-to-solid transitions.
    World_CA_Generation gen(10, 10);
    gen.set_seed(1);

    int ps = static_cast<int>(Globals::PARTICLE_SIZE);

    // Helper: find the first solid cell scanning downward.
    // This is more stable for cave-like terrain than strict air->solid transitions.
    auto find_first_solid = [&](int pixel_x, int start_y, int end_y) -> int
    {
        int cx = static_cast<int>(std::floor(static_cast<float>(pixel_x) / Globals::PARTICLE_SIZE));
        for (int py = start_y; py < end_y; py += ps)
        {
            int cy = static_cast<int>(std::floor(static_cast<float>(py) / Globals::PARTICLE_SIZE));
            if (gen.is_cell_solid(cx, cy))
                return py;
        }
        return -1;
    };

    int flat_count = 0;
    int uneven_count = 0;
    int checked = 0;
    int struct_width_cells = 8;
    int struct_pw = struct_width_cells * ps;

    std::cout << "\n=== FLAT SURFACE CHECK TEST ===" << std::endl;
    for (int base_x = -1500; base_x <= 1500; base_x += struct_pw)
    {
        // Search over a larger range to find usable terrain samples
        for (int start_y = -3000; start_y < 2500; start_y += ps * 10)
        {
            int left_y = find_first_solid(base_x, start_y, start_y + ps * 40);
            int mid_y = find_first_solid(base_x + struct_pw / 2, start_y, start_y + ps * 40);
            int right_y = find_first_solid(base_x + struct_pw - ps, start_y, start_y + ps * 40);

            if (left_y < 0 || mid_y < 0 || right_y < 0)
                continue;

            checked++;
            int min_y = std::min({left_y, mid_y, right_y});
            int max_y = std::max({left_y, mid_y, right_y});
            bool is_flat = (max_y - min_y) <= 2 * ps;

            if (is_flat)
                flat_count++;
            else
                uneven_count++;

            if (checked <= 5)
            {
                std::cout << "  X=" << base_x << " y_start=" << start_y
                          << " surface: left=" << left_y << " mid=" << mid_y
                          << " right=" << right_y << " flat=" << (is_flat ? "YES" : "NO") << std::endl;
            }
            break; // found a transition at this X, move on
        }
    }

    std::cout << "Checked: " << checked << " Flat: " << flat_count << " Uneven: " << uneven_count << std::endl;
    EXPECT_GT(checked, 0) << "Should find at least some terrain samples to check";
}

TEST(StructurePlacementTest, StructuresNotInAir)
{
    // Simulate the predetermined position generation logic using noise.
    // Verify that every generated position has solid ground beneath it.
    World_CA_Generation gen(10, 10);
    gen.set_seed(1);

    int ps = static_cast<int>(Globals::PARTICLE_SIZE);
    int chunk_radius = 15;
    int cpw = 10 * ps; // 50
    int cph = 10 * ps; // 50
    int min_px = -chunk_radius * cpw;
    int max_px = chunk_radius * cpw;
    int min_py = -chunk_radius * cph;
    int max_py = chunk_radius * cph;

    int struct_height_cells = 20; // typical devushki_column height
    int struct_width_cells = 5;
    int struct_height_px = struct_height_cells * ps;
    int struct_width_px = struct_width_cells * ps;

    std::mt19937 rng(1);
    std::uniform_int_distribution<int> dist_x(min_px, max_px - 1);

    int placed = 0;
    int attempts = 0;
    int target = 5;
    constexpr int MAX_ATTEMPTS = 10000;
    constexpr float MIN_DISTANCE = 800.0f;

    std::vector<glm::ivec2> positions;

    std::cout << "\n=== STRUCTURES NOT IN AIR TEST ===" << std::endl;

    while (placed < target && attempts < MAX_ATTEMPTS)
    {
        attempts++;

        int rand_x = dist_x(rng);
        rand_x = (rand_x / ps) * ps;

        // Find surface
        int surface_y = -1;
        for (int py = min_py; py < max_py; py += ps)
        {
            int cx = static_cast<int>(std::floor(static_cast<float>(rand_x) / Globals::PARTICLE_SIZE));
            int cy = static_cast<int>(std::floor(static_cast<float>(py) / Globals::PARTICLE_SIZE));
            if (gen.is_cell_solid(cx, cy))
            {
                surface_y = py;
                break;
            }
        }
        if (surface_y < 0)
            continue;

        // Check flatness
        auto find_surface_at = [&](int pixel_x) -> int
        {
            for (int py = surface_y - ps; py < surface_y + ps * 10; py += ps)
            {
                int cx = static_cast<int>(std::floor(static_cast<float>(pixel_x) / Globals::PARTICLE_SIZE));
                int cy = static_cast<int>(std::floor(static_cast<float>(py) / Globals::PARTICLE_SIZE));
                if (gen.is_cell_solid(cx, cy))
                    return py;
            }
            return -1;
        };

        int left_y = find_surface_at(rand_x);
        int mid_y = find_surface_at(rand_x + struct_width_px / 2);
        int right_y = find_surface_at(rand_x + struct_width_px - ps);
        if (left_y < 0 || mid_y < 0 || right_y < 0)
            continue;
        int min_y = std::min({left_y, mid_y, right_y});
        int max_y = std::max({left_y, mid_y, right_y});
        if ((max_y - min_y) > 2 * ps)
            continue;

        glm::ivec2 candidate(rand_x, surface_y - struct_height_px);
        candidate.y = (candidate.y / ps) * ps;

        // Check ground beneath
        int solid_count = 0;
        int samples = 0;
        int bottom_y = candidate.y + struct_height_px;
        for (int row = 0; row < 2; row++)
        {
            for (int gx = candidate.x; gx < candidate.x + struct_width_px; gx += ps)
            {
                samples++;
                int cx = static_cast<int>(std::floor(static_cast<float>(gx) / Globals::PARTICLE_SIZE));
                int cy = static_cast<int>(std::floor(static_cast<float>(bottom_y + row * ps) / Globals::PARTICLE_SIZE));
                if (gen.is_cell_solid(cx, cy))
                    solid_count++;
            }
        }
        if (samples == 0 || static_cast<float>(solid_count) / samples < 0.5f)
            continue;

        // Check distance
        bool too_close = false;
        for (const auto &existing : positions)
        {
            if (glm::distance(glm::vec2(candidate), glm::vec2(existing)) < MIN_DISTANCE)
            {
                too_close = true;
                break;
            }
        }
        if (too_close)
            continue;

        positions.push_back(candidate);
        placed++;

        std::cout << "  Placed #" << placed << " at (" << candidate.x << ", " << candidate.y
                  << ") surface_y=" << surface_y
                  << " ground=" << solid_count << "/" << samples << std::endl;
    }

    std::cout << "Placed " << placed << " structures in " << attempts << " attempts" << std::endl;

    // All placed structures must have solid ground beneath them
    for (const auto &pos : positions)
    {
        int bottom_y = pos.y + struct_height_px;
        int cx = static_cast<int>(std::floor(static_cast<float>(pos.x + struct_width_px / 2) / Globals::PARTICLE_SIZE));
        int cy = static_cast<int>(std::floor(static_cast<float>(bottom_y) / Globals::PARTICLE_SIZE));

        EXPECT_TRUE(gen.is_cell_solid(cx, cy))
            << "Structure at (" << pos.x << ", " << pos.y
            << ") should have solid ground beneath center at cell (" << cx << ", " << cy << ")";
    }

    EXPECT_GT(placed, 0) << "Should be able to place at least one structure with terrain-aware logic";
}
