#include <gtest/gtest.h>
#include <iostream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <cstdlib>

#include "engine/world/structure.hpp"
#include "engine/player/entity_manager.hpp"
#include "engine/player/entity.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/world/world_ca_generation.hpp"
#include "others/GLOBALS.hpp"

// ===========================================
// DEVUSHKI SPAWN ON STRUCTURE TESTS
// ===========================================

// Test that StructureSpawner correctly tracks placed structures
TEST(DevushkiSpawnTest, StructureSpawnerTracksPlacedStructures)
{
    StructureSpawner spawner;

    // Initially empty
    EXPECT_TRUE(spawner.get_placed_structures().empty());

    // Add a devushki_column blueprint
    Structure devushki_col("devushki_column", 5, 20);
    devushki_col.fill_rect(0, 0, 5, 20, Particle_Type::STONE);
    spawner.add_blueprint("devushki_column", devushki_col);

    // Verify blueprint was added
    Structure *bp = spawner.get_blueprint("devushki_column");
    ASSERT_NE(bp, nullptr);
    EXPECT_EQ(bp->get_name(), "devushki_column");
}

// Test that placed_structures can be accessed for spawning entities
TEST(DevushkiSpawnTest, PlacedStructuresAccessible)
{
    StructureSpawner spawner;

    // Get placed structures reference
    const auto &placed = spawner.get_placed_structures();

    // Should be accessible (even if empty)
    EXPECT_TRUE(placed.empty());
}

// Test loading devushki_column from image
TEST(DevushkiSpawnTest, DevushkiColumnImageLoading)
{
    Structure s = ImageStructureLoader::load_from_image("../structure_images/devushki_column.png");

    std::cout << "\n=== DEVUSHKI_COLUMN IMAGE LOADING ===" << std::endl;
    std::cout << "Structure name: " << s.get_name() << std::endl;
    std::cout << "Structure size (particles): " << s.get_width() << " x " << s.get_height() << std::endl;
    std::cout << "Structure size (pixels): " << s.get_width() * Globals::PARTICLE_SIZE
              << " x " << s.get_height() * Globals::PARTICLE_SIZE << std::endl;
    std::cout << "Solid cells: " << s.count_solid_cells() << std::endl;

    EXPECT_GT(s.get_width(), 0) << "devushki_column should load with non-zero width";
    EXPECT_GT(s.get_height(), 0) << "devushki_column should load with non-zero height";
}

// Test position hashing for spawned devushki tracking
TEST(DevushkiSpawnTest, PositionHashingConsistency)
{
    // Simulate the hashing used in Entity_Manager::check_and_spawn_devushki_on_structures
    glm::ivec2 pos1(100, 200);
    glm::ivec2 pos2(100, 200);
    glm::ivec2 pos3(150, 200);

    int hash1 = pos1.x * 73856093 ^ pos1.y * 19349663;
    int hash2 = pos2.x * 73856093 ^ pos2.y * 19349663;
    int hash3 = pos3.x * 73856093 ^ pos3.y * 19349663;

    // Same position should give same hash
    EXPECT_EQ(hash1, hash2);

    // Different position should give different hash (usually)
    EXPECT_NE(hash1, hash3);

    std::cout << "\n=== POSITION HASH TEST ===" << std::endl;
    std::cout << "Position (100, 200) hash: " << hash1 << std::endl;
    std::cout << "Position (150, 200) hash: " << hash3 << std::endl;
}

// Diagnostic test to understand spawn center calculation
TEST(DevushkiSpawnTest, SpawnCenterCalculation)
{
    // Load actual devushki_column structure
    Structure s = ImageStructureLoader::load_from_image("../structure_images/devushki_column.png");

    int struct_width_px = static_cast<int>(s.get_width() * Globals::PARTICLE_SIZE);
    int struct_height_px = static_cast<int>(s.get_height() * Globals::PARTICLE_SIZE);

    // Simulate a structure placed at position (500, 300)
    glm::ivec2 structure_pos(500, 300);

    // Calculate center-top spawn position (as done in Entity_Manager)
    glm::ivec2 spawn_pos;
    spawn_pos.x = structure_pos.x + struct_width_px / 2;
    spawn_pos.y = structure_pos.y; // top of structure

    std::cout << "\n=== SPAWN CENTER CALCULATION ===" << std::endl;
    std::cout << "Structure position: (" << structure_pos.x << ", " << structure_pos.y << ")" << std::endl;
    std::cout << "Structure size (px): " << struct_width_px << " x " << struct_height_px << std::endl;
    std::cout << "Spawn position (center-top): (" << spawn_pos.x << ", " << spawn_pos.y << ")" << std::endl;

    // Spawn should be horizontally centered
    EXPECT_EQ(spawn_pos.x, structure_pos.x + struct_width_px / 2);
    // Spawn should be at top of structure
    EXPECT_EQ(spawn_pos.y, structure_pos.y);
}

// Test DevushkiObjective configuration
TEST(DevushkiSpawnTest, DevushkiObjectiveConfig)
{
    DevushkiObjective obj;

    // Check defaults
    EXPECT_EQ(obj.total_to_collect, 5);
    EXPECT_EQ(obj.collected, 0);
    EXPECT_EQ(obj.collect_range, 50.0f);
    EXPECT_FALSE(obj.objective_active);
    EXPECT_FALSE(obj.objective_complete);

    // Simulate objective activation
    obj.objective_active = true;
    obj.total_to_collect = 3;

    EXPECT_TRUE(obj.objective_active);
    EXPECT_EQ(obj.total_to_collect, 3);
}

// ===========================================
// INTEGRATION TEST: Verify structure placement triggers devushki spawn
// ===========================================

// Test that record_placed_structure correctly adds to placed_structures
TEST(DevushkiSpawnTest, RecordPlacedStructure)
{
    StructureSpawner spawner;

    // Initially empty
    EXPECT_TRUE(spawner.get_placed_structures().empty());

    // Record a devushki_column structure
    glm::ivec2 pos(500, 300);
    spawner.record_placed_structure(pos, "devushki_column");

    // Should now have one entry
    EXPECT_EQ(spawner.get_placed_structures().size(), 1);

    const auto &placed = spawner.get_placed_structures();
    EXPECT_EQ(placed[0].position, pos);
    EXPECT_EQ(placed[0].name, "devushki_column");

    std::cout << "\n=== RECORD_PLACED_STRUCTURE TEST ===" << std::endl;
    std::cout << "Recorded structure at (" << placed[0].position.x << ", " << placed[0].position.y << ")" << std::endl;
    std::cout << "Structure name: " << placed[0].name << std::endl;
}

// Test multiple structure placement tracking
TEST(DevushkiSpawnTest, MultipleStructureTracking)
{
    StructureSpawner spawner;

    // Record multiple structures
    spawner.record_placed_structure(glm::ivec2(100, 200), "devushki_column");
    spawner.record_placed_structure(glm::ivec2(300, 400), "devushki_column");
    spawner.record_placed_structure(glm::ivec2(500, 600), "platform");

    EXPECT_EQ(spawner.get_placed_structures().size(), 3);

    // Count devushki_column entries
    int devushki_count = 0;
    for (const auto &ps : spawner.get_placed_structures())
    {
        if (ps.name == "devushki_column")
            devushki_count++;
    }

    EXPECT_EQ(devushki_count, 2);
    std::cout << "\n=== MULTIPLE STRUCTURE TRACKING ===" << std::endl;
    std::cout << "Total structures: " << spawner.get_placed_structures().size() << std::endl;
    std::cout << "Devushki columns: " << devushki_count << std::endl;
}

TEST(DevushkiSpawnTest, RealMapAttemptsToPlaceDevushkiColumn)
{
    World world;
    world.set_devushki_column_spawn_count(1);

    auto *chunks = world.get_chunks();
    auto *world_gen = world.get_world_gen();
    ASSERT_NE(chunks, nullptr);
    ASSERT_NE(world_gen, nullptr);
    const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);

    StructureSpawner &spawner = world.get_structure_spawner();
    Structure *devushki_bp = spawner.get_blueprint("devushki_column");
    ASSERT_NE(devushki_bp, nullptr);

    const auto &entries = spawner.get_predetermined_entries();
    const auto entry_it = std::find_if(
        entries.begin(),
        entries.end(),
        [](const auto &entry)
        { return entry.structure_name == "devushki_column"; });
    ASSERT_NE(entry_it, entries.end());

    const glm::ivec2 target_pos = entry_it->target_pos;
    const int chunk_pixel_w = chunk_dims.x * ps;
    const int chunk_pixel_h = chunk_dims.y * ps;
    const glm::ivec2 target_chunk(
        static_cast<int>(std::floor(static_cast<float>(target_pos.x) / chunk_pixel_w)),
        static_cast<int>(std::floor(static_cast<float>(target_pos.y) / chunk_pixel_h)));

    const int min_chunk_x = target_chunk.x - 8;
    const int max_chunk_x = target_chunk.x + 8;
    const int min_chunk_y = target_chunk.y - 8;
    const int max_chunk_y = target_chunk.y + 8;
    for (int cy = min_chunk_y; cy <= max_chunk_y; ++cy)
    {
        for (int cx = min_chunk_x; cx <= max_chunk_x; ++cx)
        {
            glm::ivec2 coords(cx, cy);
            auto chunk = std::make_unique<Chunk>(coords, chunk_dims.x, chunk_dims.y);
            world_gen->generate_chunk_with_biome(chunk.get());
            chunks->emplace(coords, std::move(chunk));
        }
    }

    const int struct_w_px = devushki_bp->get_width() * ps;
    const int struct_h_px = devushki_bp->get_height() * ps;

    // Prepare one guaranteed valid placement area in the generated map:
    // empty footprint + empty air above + solid support below.
    const int desired_x = target_pos.x;
    const int desired_y = target_pos.y;
    const int ground_y = desired_y + struct_h_px;
    const int loaded_min_y = min_chunk_y * chunk_dims.y * ps;

    for (int x = desired_x; x < desired_x + struct_w_px; x += ps)
    {
        for (int y = loaded_min_y; y < ground_y; y += ps)
        {
            world.place_custom_particle(glm::ivec2(x, y), Particle());
        }
        world.place_static_particle(glm::ivec2(x, ground_y), Particle_Type::STONE);
    }

    spawner.try_place_pending_structures(target_chunk);

    const auto &placed_structures = spawner.get_placed_structures();
    const int devushki_columns = static_cast<int>(std::count_if(
        placed_structures.begin(),
        placed_structures.end(),
        [](const auto &entry)
        { return entry.name == "devushki_column"; }));

    EXPECT_GE(world.get_chunks_size(), 100);
    EXPECT_GE(devushki_columns, 1)
        << "Expected at least one devushki_column to be placed in generated map terrain";
}

TEST(DevushkiSpawnTest, BaseFillCreatesSupportUnderStructure)
{
    World world;
    auto *chunks = world.get_chunks();
    auto *world_gen = world.get_world_gen();
    ASSERT_NE(chunks, nullptr);
    ASSERT_NE(world_gen, nullptr);

    const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);

    for (int cy = -6; cy <= 6; ++cy)
    {
        for (int cx = -6; cx <= 6; ++cx)
        {
            glm::ivec2 coords(cx, cy);
            auto chunk = std::make_unique<Chunk>(coords, chunk_dims.x, chunk_dims.y);
            world_gen->generate_chunk_with_biome(chunk.get());
            chunks->emplace(coords, std::move(chunk));
        }
    }

    Structure support_test("support_test", 4, 3);
    support_test.fill_rect(0, 0, 4, 3, Particle_Type::STONE);

    const glm::ivec2 place_pos(120, 120);
    const int base_y = place_pos.y + support_test.get_height() * ps;

    auto cell_is_empty = [&](int world_x, int world_y) -> bool
    {
        const int chunk_pixel_w = chunk_dims.x * ps;
        const int chunk_pixel_h = chunk_dims.y * ps;

        glm::ivec2 chunk_pos(
            static_cast<int>(std::floor(static_cast<float>(world_x) / chunk_pixel_w)),
            static_cast<int>(std::floor(static_cast<float>(world_y) / chunk_pixel_h)));
        Chunk *chunk = world.get_chunk(chunk_pos);
        EXPECT_NE(chunk, nullptr);
        if (!chunk)
            return true;

        int offset_x = world_x - chunk_pos.x * chunk_pixel_w;
        int offset_y = world_y - chunk_pos.y * chunk_pixel_h;
        int cell_x = offset_x / ps;
        int cell_y = offset_y / ps;
        if (cell_x < 0)
            cell_x += chunk_dims.x;
        if (cell_y < 0)
            cell_y += chunk_dims.y;

        return chunk->is_empty(cell_x, cell_y);
    };

    for (int sx = 0; sx < support_test.get_width(); ++sx)
    {
        const int x = place_pos.x + sx * ps;
        for (int depth = 0; depth < 6; ++depth)
        {
            world.place_custom_particle(glm::ivec2(x, base_y + depth * ps), Particle());
        }
    }

    StructureSpawner &spawner = world.get_structure_spawner();
    ASSERT_TRUE(spawner.place_structure(support_test, place_pos));

    for (int sx = 0; sx < support_test.get_width(); ++sx)
    {
        const int x = place_pos.x + sx * ps;
        for (int depth = 0; depth < 6; ++depth)
        {
            EXPECT_FALSE(cell_is_empty(x, base_y + depth * ps));
        }
    }
}

TEST(DevushkiSpawnTest, BaseFillStopsAtExistingGround)
{
    World world;
    auto *chunks = world.get_chunks();
    auto *world_gen = world.get_world_gen();
    ASSERT_NE(chunks, nullptr);
    ASSERT_NE(world_gen, nullptr);

    const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
    const int ps = static_cast<int>(Globals::PARTICLE_SIZE);

    for (int cy = -6; cy <= 6; ++cy)
    {
        for (int cx = -6; cx <= 6; ++cx)
        {
            glm::ivec2 coords(cx, cy);
            auto chunk = std::make_unique<Chunk>(coords, chunk_dims.x, chunk_dims.y);
            world_gen->generate_chunk_with_biome(chunk.get());
            chunks->emplace(coords, std::move(chunk));
        }
    }

    Structure support_test("support_test", 3, 3);
    support_test.fill_rect(0, 0, 3, 3, Particle_Type::STONE);

    const glm::ivec2 place_pos(180, 140);
    const int base_y = place_pos.y + support_test.get_height() * ps;

    auto cell_is_empty = [&](int world_x, int world_y) -> bool
    {
        const int chunk_pixel_w = chunk_dims.x * ps;
        const int chunk_pixel_h = chunk_dims.y * ps;

        glm::ivec2 chunk_pos(
            static_cast<int>(std::floor(static_cast<float>(world_x) / chunk_pixel_w)),
            static_cast<int>(std::floor(static_cast<float>(world_y) / chunk_pixel_h)));
        Chunk *chunk = world.get_chunk(chunk_pos);
        EXPECT_NE(chunk, nullptr);
        if (!chunk)
            return true;

        int offset_x = world_x - chunk_pos.x * chunk_pixel_w;
        int offset_y = world_y - chunk_pos.y * chunk_pixel_h;
        int cell_x = offset_x / ps;
        int cell_y = offset_y / ps;
        if (cell_x < 0)
            cell_x += chunk_dims.x;
        if (cell_y < 0)
            cell_y += chunk_dims.y;

        return chunk->is_empty(cell_x, cell_y);
    };

    for (int sx = 0; sx < support_test.get_width(); ++sx)
    {
        const int x = place_pos.x + sx * ps;
        for (int depth = 0; depth < 6; ++depth)
        {
            world.place_custom_particle(glm::ivec2(x, base_y + depth * ps), Particle());
        }
        world.place_static_particle(glm::ivec2(x, base_y + 2 * ps), Particle_Type::STONE);
    }

    StructureSpawner &spawner = world.get_structure_spawner();
    ASSERT_TRUE(spawner.place_structure(support_test, place_pos));

    for (int sx = 0; sx < support_test.get_width(); ++sx)
    {
        const int x = place_pos.x + sx * ps;

        EXPECT_FALSE(cell_is_empty(x, base_y + 0 * ps));
        EXPECT_FALSE(cell_is_empty(x, base_y + 1 * ps));
        EXPECT_FALSE(cell_is_empty(x, base_y + 2 * ps));
        EXPECT_TRUE(cell_is_empty(x, base_y + 3 * ps));
    }
}

TEST(DevushkiSpawnTest, DevushkiColumnsPlacedBeforeGameplayStarts)
{
    const char *seed_env_name = "MORCIATKO_WORLD_SEED";
    const char *existing_seed = std::getenv(seed_env_name);
    std::string saved_seed = existing_seed ? existing_seed : "";

    setenv(seed_env_name, "1337", 1);

    constexpr int requested_columns = 4;
    World world;
    world.set_devushki_column_spawn_count(requested_columns);

    if (existing_seed)
    {
        setenv(seed_env_name, saved_seed.c_str(), 1);
    }
    else
    {
        unsetenv(seed_env_name);
    }

    const auto &entries = world.get_structure_spawner().get_predetermined_entries();
    const int configured_columns = static_cast<int>(std::count_if(
        entries.begin(),
        entries.end(),
        [](const auto &entry)
        { return entry.structure_name == "devushki_column"; }));

    const int placed_columns = static_cast<int>(std::count_if(
        entries.begin(),
        entries.end(),
        [](const auto &entry)
        { return entry.structure_name == "devushki_column" && entry.placed; }));

    const auto spawn_positions = world.get_devushki_column_spawn_positions();

    ASSERT_EQ(configured_columns, requested_columns);
    EXPECT_EQ(placed_columns, configured_columns)
        << "All devushki columns should have concrete placed positions before gameplay starts";
    EXPECT_EQ(static_cast<int>(spawn_positions.size()), placed_columns)
        << "Spawn positions should match already placed devushki columns";
}

TEST(DevushkiSpawnTest, ObjectiveDevushkiSpawnsAndStaysOnColumn)
{
    World world;
    Entity_Manager manager;
    manager.set_spawn_enabled(false);
    manager.set_world(&world);

    Player *player = manager.get_player();
    ASSERT_NE(player, nullptr);
    player->set_position(glm::ivec2(-5000, -5000)); // keep player outside follow range

    Structure *devushki_col = world.get_image_structure("devushki_column");
    ASSERT_NE(devushki_col, nullptr);

    const glm::ivec2 chunk_dims = world.get_chunk_dimensions();
    const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
    const int column_width_px = devushki_col->get_width() * ps;
    const int column_height_px = devushki_col->get_height() * ps;
    const glm::ivec2 column_top_left(200, 140);

    auto *chunks = world.get_chunks();
    auto *world_gen = world.get_world_gen();
    ASSERT_NE(chunks, nullptr);
    ASSERT_NE(world_gen, nullptr);

    for (int cy = -8; cy <= 8; ++cy)
    {
        for (int cx = -8; cx <= 8; ++cx)
        {
            const glm::ivec2 coords(cx, cy);
            if (world.get_chunk(coords))
                continue;

            auto chunk = std::make_unique<Chunk>(coords, chunk_dims.x, chunk_dims.y);
            world_gen->generate_chunk_with_biome(chunk.get());
            chunks->emplace(coords, std::move(chunk));
        }
    }

    for (int y = column_top_left.y; y < column_top_left.y + column_height_px + 8 * ps; y += ps)
    {
        for (int x = column_top_left.x; x < column_top_left.x + column_width_px; x += ps)
        {
            world.place_static_particle(glm::ivec2(x, y), Particle_Type::STONE);
        }
    }

    manager.set_devushki_objective_count(1);
    manager.spawn_devushki_objective(1, 2000.0f);
    world.get_structure_spawner().record_placed_structure(column_top_left, "devushki_column");

    manager.update(1.0f / 60.0f);

    ASSERT_EQ(manager.get_devushki_count(), 1);

    Devushki *spawned = nullptr;
    auto *entities = manager.get_all_entities();
    ASSERT_NE(entities, nullptr);
    for (auto &[id, entity] : *entities)
    {
        if (entity && entity->type == Entity_Type::DEVUSHKI)
        {
            spawned = static_cast<Devushki *>(entity.get());
            break;
        }
    }
    ASSERT_NE(spawned, nullptr);

    auto has_support_below = [&](const Devushki *d) -> bool
    {
        const int left = d->coords.x - d->hitbox_dimensions_half.x;
        const int right = d->coords.x + d->hitbox_dimensions_half.x;
        const int support_y = d->coords.y + d->hitbox_dimensions_half.y + ps;

        for (int x = left; x <= right; x += ps)
        {
            if (d->is_solid_at(x, support_y))
                return true;
        }

        return d->is_solid_at(right, support_y);
    };

    EXPECT_FALSE(spawned->check_collision_at(spawned->coords));
    EXPECT_TRUE(has_support_below(spawned));
    EXPECT_GE(spawned->coords.x, column_top_left.x);
    EXPECT_LE(spawned->coords.x, column_top_left.x + column_width_px);

    const glm::ivec2 initial_pos = spawned->coords;
    for (int i = 0; i < 600; ++i)
    {
        manager.update(1.0f / 60.0f);
    }

    EXPECT_LE(std::abs(spawned->coords.x - initial_pos.x), ps * 2)
        << "Devushki should stay anchored on the column when player is out of range";
    EXPECT_LE(spawned->coords.y, initial_pos.y + ps)
        << "Devushki should not fall off the column while idling";
    EXPECT_TRUE(has_support_below(spawned));
    EXPECT_NE(spawned->get_npc_ai_state(), NPC_AI_State::WANDER);
}

// This test documents how the system should work:
// 1. spawn_devushki_objective() sets objective_active = true and stores sprite_name
// 2. When a devushki_column structure is placed, it's added to placed_structures
// 3. check_and_spawn_devushki_on_structures() (called from update) finds new structures
// 4. Devushki are created at the center-top of each new structure

TEST(DevushkiSpawnTest, SpawnSystemDocumentation)
{
    std::cout << "\n=== DEVUSHKI SPAWN SYSTEM FLOW ===" << std::endl;
    std::cout << "1. Game calls entity_manager.spawn_devushki_objective(count, radius, sprite)" << std::endl;
    std::cout << "   - Sets objective_active = true" << std::endl;
    std::cout << "   - Stores devushki_sprite_name" << std::endl;
    std::cout << "   - Clears spawned_devushki_positions set" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "2. World generates chunks, StructureSpawner::try_spawn_in_chunk() runs" << std::endl;
    std::cout << "   - Checks spawn rules for devushki_column" << std::endl;
    std::cout << "   - If conditions met, places structure and adds to placed_structures" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "3. Each frame, entity_manager.update() calls check_and_spawn_devushki_on_structures()" << std::endl;
    std::cout << "   - Iterates placed_structures looking for devushki_column entries" << std::endl;
    std::cout << "   - Checks position hash against spawned_devushki_positions" << std::endl;
    std::cout << "   - If new, creates Devushki at center-top of structure" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "POTENTIAL ISSUES TO CHECK:" << std::endl;
    std::cout << "- Is objective_active set to true?" << std::endl;
    std::cout << "- Is world pointer valid in Entity_Manager?" << std::endl;
    std::cout << "- Is devushki_column blueprint registered in StructureSpawner?" << std::endl;
    std::cout << "- Are spawn rules set up for devushki_column?" << std::endl;
    std::cout << "- Is check_and_spawn_devushki_on_structures() being called?" << std::endl;

    SUCCEED(); // Documentation test
}
