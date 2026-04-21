# Store Structure Placement Redesign (Minecraft-Style)

## Goal

Build a deterministic, scalable, chunk-driven structure system for stores that does not degrade over long sessions.

## Why the current model degrades

Current store placement keeps a global list of pending entries and scans it repeatedly each update.
As explored chunks grow, desired stores grow, pending work grows, and per-frame scan/attempt cost grows.

Observed code paths:

- Store demand grows from explored chunk count in src/world/structure.cpp (`try_place_pending_structures`)
- Store placement is called frequently from world update in src/world/world.cpp (`update`)
- Global pending vector is repeatedly traversed in src/world/structure.cpp (`try_place_pending_structures`)

This creates an unbounded backlog model.

## Minecraft pattern to copy

Minecraft uses structure sets with deterministic placement and generation stages:

1. Structure placement positions are deterministic from seed + spacing + separation + salt (random_spread).
2. Generation is chunk lifecycle driven (structure starts/references/features), not global queue scans.
3. Chunks store references so only relevant nearby structure pieces are handled per chunk.
4. Biome/terrain filters can reject attempts without creating unbounded global retries.

Important concepts to mirror:

- Region grid (spacing/separation), one candidate per region.
- Candidate is pure function of world seed + region coordinates + salt.
- Placement attempt is local to chunk/region events and bounded by budget.
- Persistent per-region state prevents repeated expensive rework.

## New architecture

### 1) Replace global pending entries with region records

Introduce store region state keyed by region coordinates, not by growing vector index.

Data model:

- StoreRegionKey: (rx, ry)
- StoreRegionState:
  - Unknown
  - CandidateComputed
  - RejectedBiomeOrRules
  - AwaitingChunks
  - Placed
  - Exhausted
- StoreRegionData:
  - candidate_world_pos
  - candidate_chunk
  - fail_reason
  - attempts
  - placed_pos

Storage:

- unordered_map<StoreRegionKey, StoreRegionData>
- unordered_set<StoreRegionKey> active_window
- deque<StoreRegionKey> bounded work queue

### 2) Region-based deterministic candidate generation

For each region in an active ring around player:

- region size in chunks = spacing
- candidate position inside region = hash(seed, salt, rx, ry)
- enforce separation by design (candidate restricted to [0, spacing-separation) area)

No retries over random global sequence indexes.
No desired-current gap loop.

### 3) Chunk event integration (starts/references style)

When chunk loads (or becomes active):

- compute nearby region keys intersecting the chunk influence window
- enqueue only those keys not in terminal state
- process a fixed number of keys or fixed ms budget per frame

This mirrors Minecraft’s chunk-step localized work and avoids full-world scans.

### 4) Placement validation split into stages

Stage A (cheap):

- biome eligibility
- distance from origin and from existing stores (spatial hash)
- chunk availability precheck

Stage B (expensive):

- footprint collision probe
- support/terrain profile
- base fill simulation

If Stage A fails, mark region state and stop reprocessing until cooldown or world conditions change.

### 5) Deterministic cooldown and retry policy

For non-terminal failures:

- assign retry_after_chunk_gen_count
- only reconsider region when world chunk count passes threshold
- cap attempts per region

This prevents pathological re-attempt loops in bad terrain.

### 6) Store occupancy index (persistent)

Keep a chunk-space spatial index of placed stores:

- bucket size based on min_distance_between_stores
- O(1)-ish near-neighbor checks

This keeps rule checks stable as world grows.

### 7) Persistence and replay safety

Save per-region state in save data:

- placed regions
- exhausted regions
- retry schedule

On reload, behavior remains deterministic and no recomputation storm happens.

## Complexity and performance properties

Current model (global pending scan):

- Per call worst-case: O(total_pending_entries \* expensive_checks)
- Grows with explored world size

Proposed model (region-local queue):

- Per call: O(frame_budget \* local_checks)
- Independent of total explored world size in steady state
- Backlog bounded by active region window + retry queue caps

## Suggested parameters

- spacing_chunks: 32-64 for stores
- separation_chunks: 8-16
- salt: fixed per structure set (store)
- frame_key_budget: 2-8 regions/frame
- frame_time_budget_ms: 0.3-1.0 ms
- max_attempts_per_region: 2-4
- retry_chunk_delta: 128-512 generated chunks

## Migration plan

1. Add StoreRegionKey and StoreRegionData types.
2. Implement deterministic candidate generator from (seed, salt, rx, ry).
3. Add region window updater from active chunks.
4. Replace desired/current store target growth loop with region window queue.
5. Route placement through stage A/stage B pipeline with budgets.
6. Add save/load persistence for region states.
7. Remove old predetermined store entry path.

## Acceptance criteria

- Long run (30-60 min) has stable store placement cost.
- Store placement call time has flat p95 and bounded p99.
- Queue length remains bounded by configured window.
- No monotonic growth in pending tasks with world age.
- Structure placement remains deterministic for same seed and travel path policy.

## What this changes in your code

Main replacement targets:

- src/world/structure.cpp:
  - replace store section in `try_place_pending_structures`
  - remove global desired/current target growth loop
  - add region-state queue and deterministic region candidate logic
- src/world/world.cpp:
  - keep chunk event hooks, but feed region queue updater
- include/engine/world/structure.hpp:
  - introduce new store region state APIs and save/load hooks

## Notes

This is a system rewrite, not a patch. The key idea is to make store generation a deterministic region process tied to chunk lifecycle, like Minecraft structure sets, instead of a global growing backlog.
