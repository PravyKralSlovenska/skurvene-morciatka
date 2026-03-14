# Project README — AI Copilot Context

## What this project is

A **falling sand survival game** written in C++ with OpenGL. The world is made entirely of simulated particles (sand, water, stone, uranium, smoke). The player explores a procedurally generated world, fights enemies, and rescues NPCs called **devushki**.

---

## Tech stack

- **Language:** C++17
- **Rendering:** OpenGL, custom shaders, batch rendering
- **Math:** GLM (all positions are `glm::ivec2` in pixels or `glm::vec2` for physics)
- **Noise:** FastNoiseLite (world generation, biomes, caves)
- **UI:** ImGui
- **Tests:** Google Test

---

## Core concepts you must understand

### Particles and the world grid

- The world is a grid of **particles** (`Particle_Type`: EMPTY, SAND, WATER, STONE, URANIUM, SMOKE).
- Each particle is stored inside a `WorldCell`, which lives inside a `Chunk`.
- **Chunks** are 10×10 cells. Each cell is `Globals::PARTICLE_SIZE` pixels (currently **5.0f**) wide.
- So one chunk = 50×50 pixels in world space.
- **Static particles** (`is_static = true`) are terrain — they never move. Player-placed particles are non-static and can fall/flow.
- Coordinate system: pixel positions. World origin `(0, 0)` is the player spawn point. Chunks are addressed by `glm::ivec2` chunk coords (not pixel coords).

### Converting between pixels and chunk coords

```cpp
int chunk_pixel_w = chunk_width * Globals::PARTICLE_SIZE;   // 50px
int chunk_pixel_h = chunk_height * Globals::PARTICLE_SIZE;  // 50px

glm::ivec2 chunk_coord = {
    (int)floor((float)pixel_pos.x / chunk_pixel_w),
    (int)floor((float)pixel_pos.y / chunk_pixel_h)
};
```

### Chunk loading

Chunks are generated **on demand** as the player moves. Only chunks within `chunk_radius = 15` of the player are active. A chunk does not exist until `World::add_chunk()` is called for it. Never assume a chunk at a given position is loaded.

### World generation

`World_CA_Generation` generates terrain using **FastNoiseLite**. It supports 4 biomes: SANDY (desert), STONE, ICY, URANIUM. Each biome has its own cave noise. Terrain is generated chunk by chunk via `generate_chunk_with_biome()`.

### Entities

All entities inherit from `Entity`. Current types: `Player`, `Enemy`, `Devushki`, `Boss`. Managed by `Entity_Manager`. Entities use pixel coords, have hitboxes, gravity, and velocity. Collision is checked against the world particle grid.

### Devushki

Devushki are friendly NPCs the player needs to rescue. They have a simple FSM (IDLE → FOLLOW → WANDER → INTERACT). There is a `DevushkiObjective` system that tracks how many have been collected. Devushki also have a **structure** counterpart — `devushki_column` — an image-loaded structure that is placed in the world.

### Structures

Structures are 2D grids of particles loaded from PNG images (`structure_images/` folder). They are placed into the world by `StructureSpawner`. Empty (transparent) pixels are skipped on placement — they do not carve terrain. The structure `"devushki_column"` is the main one in use.

### Player / Wand

The player aims with the mouse. The wand places or deletes particles in the world at cursor position. Wand type, brush size, and selected particle type are managed via `Hotbar`.

---

## Project file structure

```
include/engine/
  world/          — World, Chunk, WorldCell, Structure, WorldGeneration, Biomes
  particle/       — Particle, FallingSandSimulation, ParticleMovement
  player/         — Entity, Player, Enemy, Devushki, Boss, EntityManager, Wand, Inventory
  renderer/       — WorldRenderer, EntitiesRenderer, UIRenderer

src/
  world/          — Implementations of all world/ headers
  renderer/       — Implementations of all renderer/ headers
  controls.cpp    — Input handling (keyboard + mouse)

tests/unit/       — Google Test unit tests

structure_images/ — PNG files, each one is a structure blueprint
others/
  GLOBALS.hpp     — Global constants (PARTICLE_SIZE, etc.)
  utils.hpp/cpp   — hash_coords, calculate_offsets, etc.
```

---

## Rules for contributing code

1. **Never hardcode pixel sizes.** Always use `Globals::PARTICLE_SIZE` and `world->get_chunk_dimensions()`.
2. **Always check if a chunk exists** before reading or writing to it. `world->get_chunk(coords)` returns `nullptr` if not loaded.
3. **Static vs non-static particles matter.** Terrain must be placed with `place_static_particle()`. Player-placed or dynamic particles use `place_particle()`. Custom image-structure particles use `place_custom_particle()`.
4. **Positions are always pixel-snapped.** When generating world-space positions, snap to `PARTICLE_SIZE` grid: `x = (x / ps) * ps`.
5. **RNG must be deterministic.** Use `std::mt19937` seeded with `hash_coords(x, y, seed)` for anything chunk-local. Use the world seed directly for world-level positions.
6. **Do not block the main thread.** Chunk generation and structure placement happen inside `add_chunk()` which is called every frame. Keep it fast.
7. **Empty structure cells are skipped, not placed.** `place_structure()` already handles this — do not add extra logic for it.

---

## Coding standards

### Commenting

Every header file must follow this structure:

**File-level comment at line 1** — one short paragraph describing what this header is responsible for, what subsystem it belongs to, and what other code depends on it:

```cpp
// world_chunk.hpp
// Defines the Chunk class — a 10x10 grid of WorldCells that makes up one tile
// of the world. Chunks are the unit of world generation, loading, and rendering.
// Used by World, World_CA_Generation, FallingSandSimulation, and StructureSpawner.
```

**Per-class comment** — one line above the class declaration describing its role:

```cpp
// Stores and manages the particle grid for one chunk of the world.
class Chunk { ... };
```

**Per-function comment** — one line above every public method describing what it does, what it returns, and any important precondition:

```cpp
// Returns true if the cell at (x, y) contains no particle (type == EMPTY).
bool is_empty(int x, int y);

// Scans column world_x downward from start_y for scan_range pixels.
// Returns the Y pixel of the first solid (non-empty) cell, or -1 if none found.
int find_surface_y(int world_x, int start_y, int scan_range) const;
```

Private helpers only need a comment if their purpose is not obvious from the name.

---

### Use of `static`

Use `static` wherever appropriate:

- **Free functions in `.cpp` files** that are not declared in any header should be marked `static` to limit their linkage to that translation unit:
  ```cpp
  // Only used inside structure.cpp — not exposed to the rest of the project.
  static int hash_chunk(int x, int y, int seed) { ... }
  ```
- **Class methods** that do not access `this` and operate only on their arguments should be `static`:
  ```cpp
  // Converts a pixel position to a chunk coordinate. No instance state needed.
  static glm::ivec2 pixel_to_chunk(const glm::ivec2 &pixel_pos, int chunk_pixel_w, int chunk_pixel_h);
  ```
- **Constants** inside a class or namespace should be `static constexpr` rather than `#define` or a magic number inline:
  ```cpp
  static constexpr float SPAWN_RADIUS = 20000.0f;
  static constexpr int   MAX_SEARCH_OFFSET_CELLS = 100;
  ```

---

### No hardcoding

Never write raw numbers that represent game configuration. Every tunable value must be a named constant in one of these places:

- `others/GLOBALS.hpp` — engine-wide constants (particle size, chunk dimensions).
- A `static constexpr` at the top of the relevant `.cpp` or inside the relevant class — for system-local constants.

Examples of what must NOT appear as a raw literal:

| Bad                  | Good                                                       |
| -------------------- | ---------------------------------------------------------- |
| `20000.0f`           | `static constexpr float DEVUSHKI_SPAWN_RADIUS = 20000.0f;` |
| `100` (search range) | `static constexpr int MAX_SEARCH_CELLS = 100;`             |
| `0.7f` (empty ratio) | `static constexpr float MIN_EMPTY_RATIO = 0.7f;`           |
| `chunk_width * 5.0f` | `chunk_width * Globals::PARTICLE_SIZE`                     |

---

### Testing

Every new non-trivial function must have a corresponding Google Test in `tests/unit/`. Rules:

- **One test file per system.** Structure tests go in `structure_test.cpp`, falling sand tests in `falling_sand_test.cpp`, etc.
- **Test the contract, not the implementation.** Test what a function guarantees (return value, side effect, precondition enforcement), not how it achieves it.
- **Cover the failure path.** For any function that can return -1, nullptr, or false, write a test that exercises that path explicitly.
- **Keep tests deterministic.** Always use a fixed seed (e.g. `seed = 1`) — never `std::random_device` or time-based seeds in tests.

Minimal test template for a new helper:

```cpp
TEST(StructureSpawnerTest, FindSurfaceYReturnsMinusOneWhenNoChunksLoaded)
{
    // Arrange
    StructureSpawner spawner;
    // No world set — should gracefully return -1, not crash.

    // Act
    int result = spawner.find_surface_y(0, -500, 1000);

    // Assert
    EXPECT_EQ(result, -1);
}

TEST(StructureSpawnerTest, FindSurfaceYFindsSolidCellAcrossChunkBoundary)
{
    // Arrange: set up a world with known terrain...
    // Act + Assert: verify surface Y is found even when it sits in a different chunk.
}
```

---

## Current world seed

`seed = 1` (hardcoded in `World` constructor for now).

---

## What is actively being worked on

- **Structure spawning system** — being rewritten. The goal is: each structure has one predetermined world position generated at startup. Placement is attempted lazily as chunks load. If the exact position is invalid, search nearby. `devushki_column` spawns on a circle of radius **20 000 px** from world origin.
- **Devushki objective** — collecting devushki from their column structures is the main gameplay loop.
- **Enemy AI** — basic FSM exists, being expanded.
