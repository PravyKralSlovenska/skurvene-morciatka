#include "engine/player/entity_manager.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include "engine/player/entity.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_biomes.hpp"
#include "engine/world/world_ca_generation.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "others/GLOBALS.hpp"

namespace
{
    static constexpr int ENEMY_SPAWN_EXCLUSION_RADIUS_PARTICLES = 50;
    static constexpr int ENEMY_RANDOM_SPAWN_ATTEMPTS = 24;

    static constexpr int COIN_DROP_RADIUS_CELLS = 3;
    static constexpr int COIN_PICKUP_RADIUS_CELLS = 8;
    static constexpr float STORE_INTERACT_RANGE_PX = 220.0f;
    static constexpr int STORE_HEAL_PRICE_GOLD = 3;
    static constexpr int STORE_HEAL_AMOUNT = 30;
    static constexpr int STORE_HEAL_ITEM_PRICE_GOLD = 10;
    static constexpr int STORE_HEAL_ITEM_PRICE_SILVER = 5;
    static constexpr int STORE_AMMO_ITEM_PRICE_GOLD = 2;
    static constexpr int STORE_WAND_ITEM_PRICE_GOLD = 20;
    static constexpr int STORE_WAND_ITEM_PRICE_SILVER = 2;
    static constexpr int STORE_COMPASS_ITEM_PRICE_GOLD = 20;
    static constexpr int STORE_COMPASS_ITEM_PRICE_SILVER = 0;
    static constexpr int SILVER_PER_GOLD = 10;
    static constexpr int AMMO_PURCHASE_AMOUNT = 20;
    static constexpr int STORE_ICON_EXTRA_UP_PX = 40;

    bool entities_overlap(const Entity *a, const Entity *b)
    {
        if (!a || !b)
            return false;

        const int left_a = a->coords.x - a->hitbox_dimensions_half.x;
        const int right_a = a->coords.x + a->hitbox_dimensions_half.x;
        const int top_a = a->coords.y - a->hitbox_dimensions_half.y;
        const int bottom_a = a->coords.y + a->hitbox_dimensions_half.y;

        const int left_b = b->coords.x - b->hitbox_dimensions_half.x;
        const int right_b = b->coords.x + b->hitbox_dimensions_half.x;
        const int top_b = b->coords.y - b->hitbox_dimensions_half.y;
        const int bottom_b = b->coords.y + b->hitbox_dimensions_half.y;

        return left_a <= right_b && right_a >= left_b && top_a <= bottom_b && bottom_a >= top_b;
    }

    float get_enemy_spawn_exclusion_radius_px()
    {
        return std::max(1.0f, static_cast<float>(ENEMY_SPAWN_EXCLUSION_RADIUS_PARTICLES) * Globals::PARTICLE_SIZE);
    }

    bool is_outside_player_spawn_exclusion(const Player *player, const glm::ivec2 &pos, float min_distance_px)
    {
        if (!player)
            return true;

        const float dx = static_cast<float>(pos.x - player->coords.x);
        const float dy = static_cast<float>(pos.y - player->coords.y);
        const float dist_sq = dx * dx + dy * dy;
        return dist_sq >= min_distance_px * min_distance_px;
    }

    bool nearly_equal(float a, float b, float eps = 0.01f)
    {
        return std::abs(a - b) <= eps;
    }

    Particle make_coin_particle(bool is_gold)
    {
        Particle coin = create_sand(false); // Dynamic so coins fall like sand.
        if (is_gold)
        {
            coin.base_color = Color(255, 215, 0, 1.0f);
        }
        else
        {
            coin.base_color = Color(192, 192, 192, 1.0f);
        }
        coin.color = coin.base_color;
        coin.set_static(false);
        return coin;
    }

    bool is_gold_coin_particle(const Particle &p)
    {
        return p.type == Particle_Type::SAND && nearly_equal(p.base_color.r, 1.0f) &&
               nearly_equal(p.base_color.g, 215.0f / 255.0f) && nearly_equal(p.base_color.b, 0.0f);
    }

    bool is_silver_coin_particle(const Particle &p)
    {
        const float silver = 192.0f / 255.0f;
        return p.type == Particle_Type::SAND && nearly_equal(p.base_color.r, silver) &&
               nearly_equal(p.base_color.g, silver) && nearly_equal(p.base_color.b, silver);
    }

    bool is_coin_particle(const Particle &p)
    {
        return is_gold_coin_particle(p) || is_silver_coin_particle(p);
    }

    int make_structure_hash(const glm::ivec2 &pos)
    {
        return pos.x * 73856093 ^ pos.y * 19349663;
    }

    int price_to_silver(int gold, int silver)
    {
        return gold * SILVER_PER_GOLD + silver;
    }

    glm::ivec2 snap_to_particle_grid(const glm::ivec2 &pos, int particle_size)
    {
        return glm::ivec2(
            static_cast<int>(std::floor(static_cast<float>(pos.x) / particle_size)) * particle_size,
            static_cast<int>(std::floor(static_cast<float>(pos.y) / particle_size)) * particle_size);
    }

    bool world_pos_to_chunk_local(World *world, const glm::ivec2 &world_pos, glm::ivec2 &out_chunk_pos, glm::ivec2 &out_local_pos)
    {
        if (!world)
            return false;

        const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        const int chunk_pixel_w = chunk_dims.x * particle_size;
        const int chunk_pixel_h = chunk_dims.y * particle_size;

        out_chunk_pos = glm::ivec2(
            static_cast<int>(std::floor(static_cast<float>(world_pos.x) / chunk_pixel_w)),
            static_cast<int>(std::floor(static_cast<float>(world_pos.y) / chunk_pixel_h)));

        int pixel_offset_x = world_pos.x - out_chunk_pos.x * chunk_pixel_w;
        int pixel_offset_y = world_pos.y - out_chunk_pos.y * chunk_pixel_h;
        out_local_pos = glm::ivec2(pixel_offset_x / particle_size, pixel_offset_y / particle_size);

        if (out_local_pos.x < 0)
            out_local_pos.x += chunk_dims.x;
        if (out_local_pos.y < 0)
            out_local_pos.y += chunk_dims.y;

        return out_local_pos.x >= 0 && out_local_pos.x < chunk_dims.x &&
               out_local_pos.y >= 0 && out_local_pos.y < chunk_dims.y;
    }

    bool ensure_chunk_generated(World *world, const glm::ivec2 &chunk_coords)
    {
        if (!world)
            return false;

        if (world->get_chunk(chunk_coords))
            return true;

        auto *chunks = world->get_chunks();
        World_CA_Generation *world_gen = world->get_world_gen();
        if (!chunks || !world_gen)
            return false;

        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        auto chunk = std::make_unique<Chunk>(chunk_coords, chunk_dims.x, chunk_dims.y);
        world_gen->generate_chunk_with_biome(chunk.get());
        chunks->emplace(chunk_coords, std::move(chunk));
        return true;
    }

    bool ensure_chunks_for_entity_probe(World *world, const Entity *entity, const glm::ivec2 &center_pos, int extra_down_cells = 0)
    {
        if (!world || !entity)
            return false;

        const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
        const glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        const int chunk_pixel_w = chunk_dims.x * ps;
        const int chunk_pixel_h = chunk_dims.y * ps;

        const int left = center_pos.x - entity->hitbox_dimensions_half.x;
        const int right = center_pos.x + entity->hitbox_dimensions_half.x;
        const int top = center_pos.y - entity->hitbox_dimensions_half.y;
        const int bottom = center_pos.y + entity->hitbox_dimensions_half.y + extra_down_cells * ps;

        const int first_chunk_x = static_cast<int>(std::floor(static_cast<float>(left) / chunk_pixel_w));
        const int last_chunk_x = static_cast<int>(std::floor(static_cast<float>(right) / chunk_pixel_w));
        const int first_chunk_y = static_cast<int>(std::floor(static_cast<float>(top) / chunk_pixel_h));
        const int last_chunk_y = static_cast<int>(std::floor(static_cast<float>(bottom) / chunk_pixel_h));

        for (int cy = first_chunk_y; cy <= last_chunk_y; ++cy)
        {
            for (int cx = first_chunk_x; cx <= last_chunk_x; ++cx)
            {
                if (!ensure_chunk_generated(world, glm::ivec2(cx, cy)))
                    return false;
            }
        }

        return true;
    }

    bool has_solid_support_below(const Entity *entity, const glm::ivec2 &position)
    {
        if (!entity || !entity->world_ref)
            return false;

        const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
        const int left = position.x - entity->hitbox_dimensions_half.x;
        const int right = position.x + entity->hitbox_dimensions_half.x;
        const int support_y = position.y + entity->hitbox_dimensions_half.y + ps;

        int sample_count = 0;
        int supported_count = 0;

        for (int x = left; x <= right; x += ps)
        {
            ++sample_count;
            if (entity->is_solid_at(x, support_y))
                ++supported_count;
        }

        ++sample_count;
        if (entity->is_solid_at(right, support_y))
            ++supported_count;

        if (sample_count <= 0)
            return false;

        const int required_supported =
            (sample_count <= 2) ? 1 : std::max(2, static_cast<int>(std::ceil(static_cast<float>(sample_count) * 0.45f)));

        const bool center_supported =
            entity->is_solid_at(position.x, support_y) ||
            entity->is_solid_at(position.x - ps, support_y) ||
            entity->is_solid_at(position.x + ps, support_y);

        return center_supported && supported_count >= required_supported;
    }

    glm::ivec2 snap_spawn_to_surface(Entity *entity, World *world, const glm::ivec2 &start_pos)
    {
        if (!entity || !world)
            return start_pos;

        const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
        static constexpr int MAX_LIFT_CELLS = 40;
        static constexpr int MAX_DROP_CELLS = 1200;

        glm::ivec2 candidate = snap_to_particle_grid(start_pos, ps);
        ensure_chunks_for_entity_probe(world, entity, candidate, 4);

        if (entity->check_collision_at(candidate))
        {
            for (int lift = 1; lift <= MAX_LIFT_CELLS; ++lift)
            {
                glm::ivec2 lifted = candidate - glm::ivec2(0, lift * ps);
                ensure_chunks_for_entity_probe(world, entity, lifted, 4);
                if (!entity->check_collision_at(lifted))
                {
                    candidate = lifted;
                    break;
                }
            }
        }

        for (int drop = 0; drop < MAX_DROP_CELLS; ++drop)
        {
            glm::ivec2 next = candidate + glm::ivec2(0, ps);
            ensure_chunks_for_entity_probe(world, entity, next, 2);
            if (entity->check_collision_at(next))
                break;

            candidate = next;
        }

        return candidate;
    }

    bool is_grounded_spawn_position(Entity *entity, World *world, const glm::ivec2 &position)
    {
        if (!entity || !world)
            return false;

        ensure_chunks_for_entity_probe(world, entity, position, 2);
        if (entity->check_collision_at(position))
            return false;

        return has_solid_support_below(entity, position);
    }

    bool is_icy_biome_world_pos(World *world, const glm::ivec2 &world_pos)
    {
        if (!world)
            return false;

        World_CA_Generation *world_gen = world->get_world_gen();
        if (!world_gen)
            return false;

        const float particle_size = static_cast<float>(Globals::PARTICLE_SIZE);
        const int world_cell_x = static_cast<int>(std::floor(static_cast<float>(world_pos.x) / particle_size));
        const int world_cell_y = static_cast<int>(std::floor(static_cast<float>(world_pos.y) / particle_size));
        const glm::vec2 world_cell_coords(
            static_cast<float>(world_cell_x),
            static_cast<float>(world_cell_y));

        return world_gen->get_biome(world_cell_coords).type == Biome_Type::ICY;
    }

    glm::ivec2 find_non_icy_spawn_position(Entity *entity, World *world, std::mt19937 &rng,
                                           const glm::ivec2 &desired_pos,
                                           int max_search_radius = 500)
    {
        if (!entity || !world)
            return desired_pos;

        ensure_chunks_for_entity_probe(world, entity, desired_pos, 8);
        glm::ivec2 valid_pos = entity->find_valid_spawn_position(desired_pos, max_search_radius);
        ensure_chunks_for_entity_probe(world, entity, valid_pos, 8);
        if (!is_icy_biome_world_pos(world, valid_pos) && entity->is_valid_spawn_position(valid_pos))
        {
            return valid_pos;
        }

        (void)rng;

        const int particle_size = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
        const int step = particle_size * 2;
        const int max_radius = std::max(max_search_radius + 2200, 2600);

        auto try_candidate = [&](const glm::ivec2 &candidate) -> bool
        {
            ensure_chunks_for_entity_probe(world, entity, candidate, 8);
            if (is_icy_biome_world_pos(world, candidate))
                return false;

            const glm::ivec2 candidate_valid = entity->find_valid_spawn_position(candidate, max_search_radius + 700);
            ensure_chunks_for_entity_probe(world, entity, candidate_valid, 8);
            if (is_icy_biome_world_pos(world, candidate_valid))
                return false;
            if (!entity->is_valid_spawn_position(candidate_valid))
                return false;

            valid_pos = candidate_valid;
            return true;
        };

        for (int radius = step; radius <= max_radius; radius += step)
        {
            for (int dx = -radius; dx <= radius; dx += step)
            {
                if (try_candidate(glm::ivec2(desired_pos.x + dx, desired_pos.y - radius)))
                    return valid_pos;
                if (try_candidate(glm::ivec2(desired_pos.x + dx, desired_pos.y + radius)))
                    return valid_pos;
            }

            for (int dy = -radius + step; dy <= radius - step; dy += step)
            {
                if (try_candidate(glm::ivec2(desired_pos.x - radius, desired_pos.y + dy)))
                    return valid_pos;
                if (try_candidate(glm::ivec2(desired_pos.x + radius, desired_pos.y + dy)))
                    return valid_pos;
            }
        }

        // Fallback keeps previous behavior if no non-icy spot was found.
        return valid_pos;
    }

    glm::ivec2 find_safe_spawn_position(Entity *entity,
                                        World *world,
                                        std::mt19937 &rng,
                                        const glm::ivec2 &desired_pos,
                                        bool require_ground_support,
                                        int max_search_radius = 2200)
    {
        if (!entity || !world)
            return desired_pos;

        const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
        const int radius_step = ps * 14;
        const int max_radius = std::max(radius_step, max_search_radius);
        glm::ivec2 resolved = snap_to_particle_grid(desired_pos, ps);

        auto try_candidate = [&](const glm::ivec2 &raw_candidate) -> bool
        {
            glm::ivec2 candidate = snap_to_particle_grid(raw_candidate, ps);

            ensure_chunks_for_entity_probe(world, entity, candidate, require_ground_support ? 6 : 0);

            if (require_ground_support)
            {
                candidate = snap_spawn_to_surface(entity, world, candidate);
                ensure_chunks_for_entity_probe(world, entity, candidate, 4);
            }

            if (entity->check_collision_at(candidate))
            {
                for (int lift = 1; lift <= 120; ++lift)
                {
                    glm::ivec2 lifted = candidate - glm::ivec2(0, lift * ps);
                    ensure_chunks_for_entity_probe(world, entity, lifted, 4);
                    if (!entity->check_collision_at(lifted))
                    {
                        candidate = require_ground_support ? snap_spawn_to_surface(entity, world, lifted) : lifted;
                        break;
                    }
                }
            }

            if (is_icy_biome_world_pos(world, candidate))
                return false;

            if (!entity->is_valid_spawn_position(candidate))
                return false;

            if (require_ground_support && !has_solid_support_below(entity, candidate))
                return false;

            resolved = candidate;
            return true;
        };

        const glm::ivec2 non_icy_seed =
            find_non_icy_spawn_position(entity, world, rng, desired_pos, std::max(500, max_search_radius / 2));

        if (try_candidate(non_icy_seed))
            return resolved;
        if (try_candidate(desired_pos))
            return resolved;

        const std::array<glm::ivec2, 8> directions = {
            glm::ivec2(1, 0),
            glm::ivec2(-1, 0),
            glm::ivec2(0, 1),
            glm::ivec2(0, -1),
            glm::ivec2(1, 1),
            glm::ivec2(-1, 1),
            glm::ivec2(1, -1),
            glm::ivec2(-1, -1)};

        for (int radius = radius_step; radius <= max_radius; radius += radius_step)
        {
            for (const auto &dir : directions)
            {
                if (try_candidate(desired_pos + dir * radius))
                    return resolved;
                if (try_candidate(non_icy_seed + dir * radius))
                    return resolved;
            }
        }

        // Last resort: still try hard to satisfy all constraints for grounded spawns.
        for (int lift = 1; lift <= 1400; ++lift)
        {
            glm::ivec2 lifted = snap_to_particle_grid(desired_pos - glm::ivec2(0, lift * ps), ps);
            ensure_chunks_for_entity_probe(world, entity, lifted, 8);
            if (!entity->is_valid_spawn_position(lifted))
                continue;

            glm::ivec2 settled = require_ground_support ? snap_spawn_to_surface(entity, world, lifted) : lifted;
            ensure_chunks_for_entity_probe(world, entity, settled, 8);

            if (is_icy_biome_world_pos(world, settled))
                continue;
            if (!entity->is_valid_spawn_position(settled))
                continue;
            if (require_ground_support && !has_solid_support_below(entity, settled))
                continue;

            return settled;
        }

        return resolved;
    }

    glm::ivec2 find_devushki_column_anchor_position(Devushki *devushki,
                                                    World *world,
                                                    const glm::ivec2 &column_top_left,
                                                    int column_width_px)
    {
        if (!devushki || !world)
            return column_top_left;

        const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));
        const int center_x = column_top_left.x + column_width_px / 2;
        const int base_spawn_y = column_top_left.y - devushki->hitbox_dimensions_half.y - 1;

        auto is_valid_anchor = [&](const glm::ivec2 &pos) -> bool
        {
            ensure_chunks_for_entity_probe(world, devushki, pos, 4);
            if (devushki->check_collision_at(pos))
                return false;
            return has_solid_support_below(devushki, pos);
        };

        glm::ivec2 centered(center_x, base_spawn_y);
        if (is_valid_anchor(centered))
            return centered;

        const int max_offset_steps = std::max(1, column_width_px / (2 * ps));
        for (int step = 1; step <= max_offset_steps; ++step)
        {
            const int offset = step * ps;

            glm::ivec2 right(center_x + offset, base_spawn_y);
            if (is_valid_anchor(right))
                return right;

            glm::ivec2 left(center_x - offset, base_spawn_y);
            if (is_valid_anchor(left))
                return left;
        }

        // Last resort: find nearby free space and settle onto nearest support.
        glm::ivec2 lifted(center_x, base_spawn_y - 6 * ps);
        glm::ivec2 free_pos = devushki->find_valid_spawn_position(lifted, 250);
        glm::ivec2 settled = snap_spawn_to_surface(devushki, world, free_pos);
        if (is_valid_anchor(settled))
            return settled;

        return centered;
    }
} // namespace

Entity_Manager::Entity_Manager()
    : rng(std::random_device{}())
{
    player = std::make_unique<Player>("Player", glm::vec2(0, 0));
}

Entity_Manager::~Entity_Manager() = default;

void Entity_Manager::set_world(World *world)
{
    this->world = world;

    if (world)
    {
        glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        chunk_pixel_width = chunk_dims.x * static_cast<int>(Globals::PARTICLE_SIZE);
        chunk_pixel_height = chunk_dims.y * static_cast<int>(Globals::PARTICLE_SIZE);

        // Set world reference on player
        if (player)
        {
            player->set_world(world);
        }

        // Set world reference on all existing entities
        for (auto &[id, entity] : entities)
        {
            entity->set_world(world);
        }
    }
}

void Entity_Manager::reset_for_new_world()
{
    remove_all_entities();

    spawn_timer = 0.0f;
    spawned_devushki_positions.clear();
    store_offers_by_structure.clear();

    devushki_objective.collected = 0;
    devushki_objective.objective_active = false;
    devushki_objective.objective_complete = false;
    devushki_objective.boss_spawned = false;

    collected_gold_coins = 0;
    collected_silver_coins = 0;
    player_ammo = 60;
    player_has_compass = true;

    if (player)
    {
        player->set_world(world);
        player->set_position(0, 0);
        player->set_velocity(0.0f, 0.0f);
        player->acceleration = glm::vec2(0.0f, 0.0f);
        player->healthpoints = player->max_healthpoints;
        player->is_alive = true;
        player->is_active = true;
        player->state = Entity_States::STILL;
    }
}

void Entity_Manager::ensure_player_valid_position()
{
    if (!player || !world)
        return;

    const glm::ivec2 origin = player->coords;
    const int ps = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));

    auto is_strict_safe_player_spawn = [&](const glm::ivec2 &pos) -> bool
    {
        ensure_chunks_for_entity_probe(world, player.get(), pos, 8);
        if (is_icy_biome_world_pos(world, pos))
            return false;
        if (!player->is_valid_spawn_position(pos))
            return false;
        if (!has_solid_support_below(player.get(), pos))
            return false;
        return true;
    };

    glm::ivec2 valid_pos = find_safe_spawn_position(player.get(), world, rng, origin, true, 3200);
    valid_pos = snap_spawn_to_surface(player.get(), world, valid_pos);

    if (!is_strict_safe_player_spawn(valid_pos))
    {
        const int radius_step = ps * 14;
        const int max_radius = 5200;
        const std::array<glm::ivec2, 8> directions = {
            glm::ivec2(1, 0),
            glm::ivec2(-1, 0),
            glm::ivec2(0, 1),
            glm::ivec2(0, -1),
            glm::ivec2(1, 1),
            glm::ivec2(-1, 1),
            glm::ivec2(1, -1),
            glm::ivec2(-1, -1)};

        bool found = false;
        for (int radius = radius_step; radius <= max_radius && !found; radius += radius_step)
        {
            for (const auto &dir : directions)
            {
                glm::ivec2 candidate = origin + dir * radius;
                candidate = find_safe_spawn_position(player.get(), world, rng, candidate, true, radius + 900);
                candidate = snap_spawn_to_surface(player.get(), world, candidate);

                if (!is_strict_safe_player_spawn(candidate))
                    continue;

                valid_pos = candidate;
                found = true;
                break;
            }
        }

        if (!found)
        {
            for (int lift = 1; lift <= 1800; ++lift)
            {
                glm::ivec2 lifted = origin - glm::ivec2(0, lift * ps);
                ensure_chunks_for_entity_probe(world, player.get(), lifted, 8);
                if (!player->is_valid_spawn_position(lifted))
                    continue;

                glm::ivec2 settled = snap_spawn_to_surface(player.get(), world, lifted);
                if (is_strict_safe_player_spawn(settled))
                {
                    valid_pos = settled;
                    found = true;
                    break;
                }
            }
        }
    }

    // Absolute fallback to prevent embedded spawns if strict criteria could not be met.
    if (!player->is_valid_spawn_position(valid_pos))
    {
        ensure_chunks_for_entity_probe(world, player.get(), origin, 12);
        glm::ivec2 free_pos = player->find_valid_spawn_position(origin, 5200);
        valid_pos = snap_spawn_to_surface(player.get(), world, free_pos);
    }

    player->set_position(valid_pos);
    player->set_velocity(0.0f, 0.0f);
}

Entity *Entity_Manager::create_entity()
{
    auto entity = std::make_unique<Entity>();
    int id = entity->get_id();
    entities[id] = std::move(entity);
    return entities[id].get();
}

Enemy *Entity_Manager::create_enemy(const glm::ivec2 &position, const std::string &sprite_name)
{
    auto enemy = std::make_unique<Enemy>();

    // Set world reference first so collision checks work
    if (world)
    {
        enemy->set_world(world);
    }

    // Resolve spawn against generated terrain/structures so enemy never starts embedded.
    glm::ivec2 valid_pos = find_safe_spawn_position(enemy.get(), world, rng, position, true);
    enemy->set_position(valid_pos);

    int id = enemy->get_id();
    Enemy *enemy_ptr = enemy.get();
    entities[id] = std::move(enemy);

    // Apply registered sprite if name provided
    if (!sprite_name.empty())
    {
        apply_sprite(enemy_ptr, sprite_name);
    }

    return enemy_ptr;
}

Enemy *Entity_Manager::create_enemy(int x, int y, const std::string &sprite_name)
{
    return create_enemy(glm::ivec2(x, y), sprite_name);
}

Devushki *Entity_Manager::create_devushki(const glm::ivec2 &position, const std::string &sprite_name)
{
    auto devushki = std::make_unique<Devushki>();

    // Set world reference first so collision checks work
    if (world)
    {
        devushki->set_world(world);
    }

    // Resolve spawn against generated terrain/structures so devushki never starts embedded.
    glm::ivec2 valid_pos = find_safe_spawn_position(devushki.get(), world, rng, position, true);
    devushki->set_position(valid_pos);
    devushki->set_home_position(valid_pos);

    int id = devushki->get_id();
    Devushki *devushki_ptr = devushki.get();
    entities[id] = std::move(devushki);

    std::string resolved_sprite_name;

    if (!sprite_name.empty() && has_sprite(sprite_name))
    {
        resolved_sprite_name = sprite_name;
    }
    else
    {
        // Support variant groups, e.g. sprite_name="devushki" => devushki_1/devushki_2/...
        const std::string group_prefix = sprite_name.empty() ? "devushki_" : (sprite_name + "_");
        std::vector<std::string> variant_names;
        variant_names.reserve(sprite_registry.size());

        for (const auto &[name, config] : sprite_registry)
        {
            if (!config.is_valid)
                continue;

            if (name.rfind(group_prefix, 0) == 0)
            {
                variant_names.push_back(name);
            }
        }

        if (!variant_names.empty())
        {
            std::sort(variant_names.begin(), variant_names.end());
            std::uniform_int_distribution<std::size_t> variant_dist(0, variant_names.size() - 1);
            resolved_sprite_name = variant_names[variant_dist(rng)];
        }
    }

    if (!resolved_sprite_name.empty())
    {
        apply_sprite(devushki_ptr, resolved_sprite_name);
    }

    return devushki_ptr;
}

Devushki *Entity_Manager::create_devushki(int x, int y, const std::string &sprite_name)
{
    return create_devushki(glm::ivec2(x, y), sprite_name);
}

Projectile *Entity_Manager::create_projectile(const glm::vec2 &position, const glm::vec2 &velocity,
                                              Particle_Type payload_type,
                                              float damage,
                                              Entity_Type owner_type)
{
    auto projectile = std::make_unique<Projectile>(position, velocity, payload_type);
    projectile->set_damage(damage);
    projectile->set_owner_type(owner_type);

    if (world)
    {
        projectile->set_world(world);
    }

    int id = projectile->get_id();
    Projectile *projectile_ptr = projectile.get();
    entities[id] = std::move(projectile);
    return projectile_ptr;
}

Boss *Entity_Manager::create_boss(const glm::ivec2 &position, const std::string &sprite_name)
{
    auto boss = std::make_unique<Boss>();

    // Set world reference first so collision checks work
    if (world)
    {
        boss->set_world(world);
    }

    // Resolve spawn against generated terrain/structures so boss never starts embedded.
    glm::ivec2 valid_pos = find_safe_spawn_position(boss.get(), world, rng, position, true);
    boss->set_position(valid_pos);
    boss->set_home_position(valid_pos);

    int id = boss->get_id();
    Boss *boss_ptr = boss.get();
    entities[id] = std::move(boss);

    // Apply registered sprite if name provided
    if (!sprite_name.empty())
    {
        apply_sprite(boss_ptr, sprite_name);
    }

    return boss_ptr;
}

Boss *Entity_Manager::create_boss(int x, int y, const std::string &sprite_name)
{
    return create_boss(glm::ivec2(x, y), sprite_name);
}

bool Entity_Manager::remove_entity(const int id)
{
    return entities.erase(id) > 0;
}

void Entity_Manager::remove_all_entities()
{
    entities.clear();
}

void Entity_Manager::remove_all_dead()
{
    for (auto it = entities.begin(); it != entities.end();)
    {
        if (!it->second->get_is_alive())
        {
            if (it->second->type == Entity_Type::ENEMY)
            {
                const Enemy *enemy = static_cast<const Enemy *>(it->second.get());
                drop_enemy_coin_particles(enemy);
            }
            it = entities.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Entity_Manager::update_entity(Entity *entity, float delta_time)
{
    if (!entity || !entity->is_active)
        return;

    entity->update(delta_time);

    // If it's an enemy, update its target to player position
    if (entity->type == Entity_Type::ENEMY && player)
    {
        Enemy *enemy = static_cast<Enemy *>(entity);
        enemy->set_target(player->coords);
    }

    // If it's a devushki, update its target to player position
    if (entity->type == Entity_Type::DEVUSHKI && player)
    {
        Devushki *devushki = static_cast<Devushki *>(entity);
        devushki->set_target(player->coords);
    }

    // If it's a boss, update its target to player position
    if (entity->type == Entity_Type::BOSS && player)
    {
        Boss *boss = static_cast<Boss *>(entity);
        boss->set_target(player->coords);
    }
}

void Entity_Manager::update()
{
    update(1.0f / 60.0f); // Default to ~60 FPS delta
}

void Entity_Manager::update(float delta_time)
{
    // Update spawner
    update_spawner(delta_time);

    // Check for newly placed structures and spawn devushki on them
    check_and_spawn_devushki_on_structures();
    update_store_offers();

    // Update player physics (gravity, collision) - input is handled separately
    if (player)
    {
        player->update_damage_timers(delta_time);
        player->update(delta_time);
    }

    // Update all entities (enemies, NPCs, etc.)
    for (auto &[id, entity] : entities)
    {
        entity->update_damage_timers(delta_time);
        update_entity(entity.get(), delta_time);
    }

    process_boss_special_actions();

    // Resolve projectile impact damage after movement updates.
    resolve_projectile_entity_hits();
    resolve_hostile_melee_hits();

    // Check devushki collection
    check_devushki_collection();

    // Spawn the objective boss once all devushki are collected.
    if (devushki_objective.objective_complete && !devushki_objective.boss_spawned)
    {
        spawn_boss_for_completed_objective();
    }

    // Remove dead entities
    remove_all_dead();

    // Collect nearby dropped coin particles.
    resolve_coin_collection();
}

void Entity_Manager::drop_enemy_coin_particles(const Enemy *enemy)
{
    if (!enemy || !world)
        return;

    const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
    const glm::ivec2 center = snap_to_particle_grid(enemy->coords, particle_size);

    std::uniform_int_distribution<int> gold_count_dist(1, 4);
    std::uniform_int_distribution<int> silver_count_dist(2, 6);
    std::uniform_int_distribution<int> offset_dist(-COIN_DROP_RADIUS_CELLS, COIN_DROP_RADIUS_CELLS);

    const int gold_count = gold_count_dist(rng);
    const int silver_count = silver_count_dist(rng);

    auto drop_coin_batch = [&](int count, bool is_gold)
    {
        Particle coin_particle = make_coin_particle(is_gold);

        for (int i = 0; i < count; ++i)
        {
            int dx = 0;
            int dy = 0;
            do
            {
                dx = offset_dist(rng);
                dy = offset_dist(rng);
            } while (dx * dx + dy * dy > COIN_DROP_RADIUS_CELLS * COIN_DROP_RADIUS_CELLS);

            glm::ivec2 drop_pos = center + glm::ivec2(dx * particle_size, dy * particle_size);
            world->place_custom_particle(drop_pos, coin_particle);
        }
    };

    drop_coin_batch(gold_count, true);
    drop_coin_batch(silver_count, false);
}

void Entity_Manager::resolve_coin_collection()
{
    if (!world || !player)
        return;

    const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
    const int pickup_radius_px = COIN_PICKUP_RADIUS_CELLS * particle_size;
    const int left = player->coords.x - player->hitbox_dimensions_half.x - pickup_radius_px;
    const int right = player->coords.x + player->hitbox_dimensions_half.x + pickup_radius_px;
    const int top = player->coords.y - player->hitbox_dimensions_half.y - pickup_radius_px;
    const int bottom = player->coords.y + player->hitbox_dimensions_half.y + pickup_radius_px;

    for (int y = top; y <= bottom; y += particle_size)
    {
        for (int x = left; x <= right; x += particle_size)
        {
            glm::ivec2 world_pos(x, y);
            glm::ivec2 chunk_pos;
            glm::ivec2 local_pos;
            if (!world_pos_to_chunk_local(world, world_pos, chunk_pos, local_pos))
                continue;

            Chunk *chunk = world->get_chunk(chunk_pos);
            if (!chunk)
                continue;

            WorldCell *cell = chunk->get_worldcell(local_pos.x, local_pos.y);
            if (!cell)
                continue;

            if (!is_coin_particle(cell->particle))
                continue;

            if (is_gold_coin_particle(cell->particle))
            {
                ++collected_gold_coins;
                chunk->set_worldcell(local_pos, Particle_Type::EMPTY, false);
                continue;
            }

            if (is_silver_coin_particle(cell->particle))
            {
                ++collected_silver_coins;
                chunk->set_worldcell(local_pos, Particle_Type::EMPTY, false);
            }
        }
    }

    normalize_currency();
}

void Entity_Manager::normalize_currency()
{
    if (collected_silver_coins >= SILVER_PER_GOLD)
    {
        const int carry_gold = collected_silver_coins / SILVER_PER_GOLD;
        collected_gold_coins += carry_gold;
        collected_silver_coins %= SILVER_PER_GOLD;
    }

    if (collected_silver_coins < 0)
        collected_silver_coins = 0;
    if (collected_gold_coins < 0)
        collected_gold_coins = 0;
}

int Entity_Manager::get_total_currency_in_silver() const
{
    return collected_gold_coins * SILVER_PER_GOLD + collected_silver_coins;
}

bool Entity_Manager::find_store_display_anchor(const Structure &store_structure, glm::ivec2 &out_anchor_cells) const
{
    int top_y = std::numeric_limits<int>::max();
    int sum_top_x = 0;
    int top_count = 0;

    for (int y = 0; y < store_structure.get_height(); ++y)
    {
        for (int x = 0; x < store_structure.get_width(); ++x)
        {
            const Particle &cell = store_structure.get_cell(x, y);
            if (cell.type == Particle_Type::EMPTY)
                continue;

            if (y < top_y)
            {
                top_y = y;
                sum_top_x = x;
                top_count = 1;
            }
            else if (y == top_y)
            {
                sum_top_x += x;
                ++top_count;
            }
        }
    }

    if (top_count <= 0)
        return false;

    out_anchor_cells.x = sum_top_x / top_count;
    out_anchor_cells.y = top_y;
    return true;
}

void Entity_Manager::update_store_offers()
{
    if (!world)
        return;

    Structure *store_structure = world->get_structure_spawner().get_blueprint("store");
    if (!store_structure)
        return;

    glm::ivec2 marker_anchor_cells(store_structure->get_width() / 2, store_structure->get_height() / 2);
    find_store_display_anchor(*store_structure, marker_anchor_cells);

    const int particle_size = static_cast<int>(Globals::PARTICLE_SIZE);
    const auto &placed_structures = world->get_structure_spawner().get_placed_structures();

    std::unordered_set<int> active_hashes;
    for (const auto &ps : placed_structures)
    {
        if (ps.name != "store" && ps.name != "devushki_store")
            continue;

        const int structure_hash = make_structure_hash(ps.position);
        active_hashes.insert(structure_hash);

        if (store_offers_by_structure.find(structure_hash) != store_offers_by_structure.end())
            continue;

        std::uniform_int_distribution<int> offer_dist(0, player_has_compass ? 4 : 5);
        Store_Offer offer;
        offer.structure_hash = structure_hash;
        offer.structure_world_pos = ps.position;
        offer.display_world_pos = glm::ivec2(
            ps.position.x + marker_anchor_cells.x * particle_size,
            ps.position.y + marker_anchor_cells.y * particle_size - STORE_ICON_EXTRA_UP_PX);

        switch (offer_dist(rng))
        {
        case 0:
            offer.type = Store_Offer_Type::HEAL;
            offer.item_name = "Heal";
            offer.icon_path = "../items/devushki_heal.png";
            offer.price_gold = STORE_HEAL_ITEM_PRICE_GOLD;
            offer.price_silver = STORE_HEAL_ITEM_PRICE_SILVER;
            break;
        case 1:
            offer.type = Store_Offer_Type::AMMO;
            offer.item_name = "Ammo";
            offer.icon_path = "../items/devushki_ammo.png";
            offer.price_gold = STORE_AMMO_ITEM_PRICE_GOLD;
            offer.price_silver = 0;
            break;
        case 2:
            offer.type = Store_Offer_Type::WAND_FIRE;
            offer.item_name = "Fire Wand";
            offer.icon_path = "builtin://wand_fire";
            offer.price_gold = STORE_WAND_ITEM_PRICE_GOLD;
            offer.price_silver = STORE_WAND_ITEM_PRICE_SILVER;
            break;
        case 3:
            offer.type = Store_Offer_Type::WAND_WOOD;
            offer.item_name = "Wood Wand";
            offer.icon_path = "builtin://wand_wood";
            offer.price_gold = STORE_WAND_ITEM_PRICE_GOLD;
            offer.price_silver = STORE_WAND_ITEM_PRICE_SILVER;
            break;
        case 4:
            offer.type = Store_Offer_Type::WAND_EMPTY;
            offer.item_name = "Empty Wand";
            offer.icon_path = "builtin://wand_empty";
            offer.price_gold = STORE_WAND_ITEM_PRICE_GOLD;
            offer.price_silver = STORE_WAND_ITEM_PRICE_SILVER;
            break;
        default:
            offer.type = Store_Offer_Type::COMPASS;
            offer.item_name = "Compass";
            offer.icon_path = "../items/devushki_compass.png";
            offer.price_gold = STORE_COMPASS_ITEM_PRICE_GOLD;
            offer.price_silver = STORE_COMPASS_ITEM_PRICE_SILVER;
            break;
        }

        store_offers_by_structure[structure_hash] = offer;
    }

    for (auto it = store_offers_by_structure.begin(); it != store_offers_by_structure.end();)
    {
        if (active_hashes.find(it->first) == active_hashes.end())
        {
            it = store_offers_by_structure.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Entity_Manager::resolve_hostile_melee_hits()
{
    if (!player || !player->get_is_alive())
        return;

    for (auto &[id, entity] : entities)
    {
        if (!entity || !entity->is_active || !entity->get_is_alive())
            continue;

        if (entity->type == Entity_Type::ENEMY)
        {
            Enemy *enemy = static_cast<Enemy *>(entity.get());
            if (enemy->get_ai_state() == AI_State::ATTACK && enemy->is_in_attack_range(player->coords))
            {
                player->take_damage(enemy->get_attack_damage());
            }
            continue;
        }

        if (entity->type == Entity_Type::BOSS)
        {
            Boss *boss = static_cast<Boss *>(entity.get());
            if (boss->get_boss_ai_state() == Boss_AI_State::ATTACK && boss->is_in_attack_range(player->coords))
            {
                player->take_damage(boss->get_attack_damage());
            }
        }
    }
}

void Entity_Manager::process_boss_special_actions()
{
    if (!player || !player->get_is_alive())
        return;

    std::vector<Projectile *> player_projectiles;
    std::vector<Boss *> bosses;
    player_projectiles.reserve(32);
    bosses.reserve(8);

    for (auto &[id, entity] : entities)
    {
        if (!entity || !entity->is_active || !entity->get_is_alive())
            continue;

        if (entity->type == Entity_Type::PROJECTILE)
        {
            Projectile *projectile = static_cast<Projectile *>(entity.get());
            if (projectile->get_owner_type() == Entity_Type::PLAYER)
            {
                player_projectiles.push_back(projectile);
            }
            continue;
        }

        if (entity->type == Entity_Type::BOSS)
        {
            bosses.push_back(static_cast<Boss *>(entity.get()));
        }
    }

    for (Boss *boss : bosses)
    {
        if (!boss)
            continue;

        for (Projectile *incoming : player_projectiles)
        {
            if (!incoming || !incoming->is_active || !incoming->get_is_alive())
                continue;

            const glm::vec2 to_boss = glm::vec2(boss->coords - incoming->coords);
            const float dist_sq = to_boss.x * to_boss.x + to_boss.y * to_boss.y;
            if (dist_sq > 260.0f * 260.0f)
                continue;

            const glm::vec2 projectile_velocity = incoming->velocity;
            const float vel_len = std::sqrt(projectile_velocity.x * projectile_velocity.x + projectile_velocity.y * projectile_velocity.y);
            if (vel_len > 0.001f)
            {
                const glm::vec2 vel_norm = projectile_velocity / vel_len;
                if (glm::dot(vel_norm, to_boss) <= 0.0f)
                    continue;
            }

            if (boss->try_teleport_dodge_from(incoming->coords, incoming->velocity))
                break;
        }

        glm::vec2 shot_origin;
        glm::vec2 shot_velocity;
        float shot_damage = 0.0f;
        Particle_Type shot_payload = Particle_Type::FIRE;

        int spawned_shots_this_tick = 0;
        while (boss->consume_pending_fireball(shot_origin, shot_velocity, shot_damage, shot_payload))
        {
            Projectile *boss_projectile = create_projectile(
                shot_origin,
                shot_velocity,
                shot_payload,
                shot_damage,
                Entity_Type::BOSS);

            if (boss_projectile)
            {
                boss_projectile->set_lifetime(4.0f);
            }

            ++spawned_shots_this_tick;
            if (spawned_shots_this_tick >= 12)
                break;
        }
    }
}

void Entity_Manager::resolve_projectile_entity_hits()
{
    for (auto &[id, entity] : entities)
    {
        if (!entity || entity->type != Entity_Type::PROJECTILE || !entity->is_active || !entity->get_is_alive())
            continue;

        Projectile *projectile = static_cast<Projectile *>(entity.get());
        const Entity_Type owner_type = projectile->get_owner_type();

        // Player is stored separately from entities map.
        if (player && player->get_is_alive() && owner_type != Entity_Type::PLAYER && entities_overlap(projectile, player.get()))
        {
            player->take_damage(projectile->get_damage());
            projectile->die();
            continue;
        }

        for (auto &[target_id, target_entity] : entities)
        {
            if (!target_entity || target_entity.get() == projectile)
                continue;
            if (!target_entity->is_active || !target_entity->get_is_alive())
                continue;
            if (target_entity->type == Entity_Type::PROJECTILE)
                continue;
            if (target_entity->type == owner_type)
                continue;

            if (entities_overlap(projectile, target_entity.get()))
            {
                target_entity->take_damage(projectile->get_damage());
                projectile->die();
                break;
            }
        }
    }
}

// ==================== Spawn System ====================

void Entity_Manager::update_spawner(float delta_time)
{
    if (!spawn_config.spawn_enabled)
        return;

    spawn_timer += delta_time;

    if (spawn_timer >= spawn_config.spawn_interval)
    {
        spawn_timer = 0.0f;

        int active_chunk_enemy_count = get_enemy_count();
        if (world)
        {
            const auto *active_chunks = world->get_active_chunks();
            if (active_chunks)
            {
                active_chunk_enemy_count = get_enemy_count_in_chunks(*active_chunks);
            }
        }

        // Check if we can spawn more enemies in currently active chunks.
        if (active_chunk_enemy_count < spawn_config.max_enemies)
        {
            spawn_random_enemy("slime");
        }
    }
}

glm::ivec2 Entity_Manager::get_random_spawn_position()
{
    if (!player)
        return {0, 0};

    const float exclusion_min_distance = get_enemy_spawn_exclusion_radius_px();
    const float min_distance = std::max(spawn_config.min_spawn_distance, exclusion_min_distance);
    const float max_distance = std::max(min_distance + 1.0f, spawn_config.max_spawn_distance);

    // Generate random angle
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * 3.14159265f);
    float angle = angle_dist(rng);

    // Generate random distance between min and max
    std::uniform_real_distribution<float> dist_dist(min_distance, max_distance);
    float distance = dist_dist(rng);

    // Calculate spawn position
    glm::ivec2 spawn_pos;
    spawn_pos.x = player->coords.x + static_cast<int>(std::cos(angle) * distance);
    spawn_pos.y = player->coords.y + static_cast<int>(std::sin(angle) * distance);

    return spawn_pos;
}

void Entity_Manager::randomize_enemy_stats(Enemy *enemy)
{
    if (!enemy)
        return;

    float diff = spawn_config.difficulty_multiplier;

    // Randomize health (40-60 base, scaled by difficulty)
    std::uniform_real_distribution<float> health_dist(40.0f, 60.0f);
    float health = health_dist(rng) * diff;
    enemy->healthpoints = health;
    enemy->max_healthpoints = health;

    // Randomize speed (40-70 base)
    std::uniform_real_distribution<float> speed_dist(40.0f, 70.0f);
    enemy->speed = speed_dist(rng);

    // Randomize detection range (150-300)
    std::uniform_real_distribution<float> detect_dist(150.0f, 300.0f);
    enemy->set_detection_range(detect_dist(rng));

    // Randomize attack damage (8-15, scaled by difficulty)
    std::uniform_real_distribution<float> damage_dist(8.0f, 15.0f);
    enemy->set_attack_damage(damage_dist(rng) * diff);

    // Randomize attack range (30-60)
    std::uniform_real_distribution<float> range_dist(30.0f, 60.0f);
    enemy->set_attack_range(range_dist(rng));

    // Random chance to add patrol points
    std::uniform_int_distribution<int> patrol_chance(0, 3);
    if (patrol_chance(rng) == 0) // 25% chance
    {
        glm::ivec2 home = enemy->coords;
        std::uniform_int_distribution<int> offset_dist(-100, 100);

        int num_points = 2 + (rng() % 3); // 2-4 patrol points
        for (int i = 0; i < num_points; i++)
        {
            enemy->add_patrol_point({home.x + offset_dist(rng),
                                     home.y + offset_dist(rng)});
        }
    }
}

Enemy *Entity_Manager::spawn_random_enemy(const std::string &sprite_name)
{
    const float exclusion_min_distance = get_enemy_spawn_exclusion_radius_px();

    for (int attempt = 0; attempt < ENEMY_RANDOM_SPAWN_ATTEMPTS; ++attempt)
    {
        glm::ivec2 spawn_pos = get_random_spawn_position();

        if (!is_outside_player_spawn_exclusion(player.get(), spawn_pos, exclusion_min_distance))
            continue;

        // Create a temporary enemy to check hitbox size for valid position.
        auto temp = std::make_unique<Enemy>();
        if (world)
        {
            temp->set_world(world);
        }
        glm::ivec2 valid_pos = temp->find_valid_spawn_position(spawn_pos);

        if (!is_outside_player_spawn_exclusion(player.get(), valid_pos, exclusion_min_distance))
            continue;

        Enemy *enemy = create_enemy(valid_pos, sprite_name);
        if (!enemy)
            continue;

        // create_enemy can adjust position for safety; enforce exclusion again on final coords.
        if (!is_outside_player_spawn_exclusion(player.get(), enemy->coords, exclusion_min_distance))
        {
            remove_entity(enemy->get_id());
            continue;
        }

        enemy->set_home_position(enemy->coords);
        randomize_enemy_stats(enemy);
        return enemy;
    }

    return nullptr;
}

// ==================== Spawn Config Setters ====================

void Entity_Manager::set_spawn_enabled(bool enabled)
{
    spawn_config.spawn_enabled = enabled;
}

void Entity_Manager::set_spawn_interval(float interval)
{
    spawn_config.spawn_interval = interval;
}

void Entity_Manager::set_max_enemies(int max)
{
    spawn_config.max_enemies = max;
}

void Entity_Manager::set_spawn_distance(float min_dist, float max_dist)
{
    const float exclusion_min_distance = get_enemy_spawn_exclusion_radius_px();
    spawn_config.min_spawn_distance = std::max(min_dist, exclusion_min_distance);
    spawn_config.max_spawn_distance = std::max(max_dist, spawn_config.min_spawn_distance + 1.0f);
}

void Entity_Manager::set_difficulty(float multiplier)
{
    spawn_config.difficulty_multiplier = multiplier;
}

SpawnConfig &Entity_Manager::get_spawn_config()
{
    return spawn_config;
}

Player *Entity_Manager::get_player()
{
    return player.get();
}

Entity *Entity_Manager::get_entity(const int id)
{
    auto it = entities.find(id);
    if (it != entities.end())
    {
        return it->second.get();
    }
    return nullptr;
}

std::unordered_map<int, std::unique_ptr<Entity>> *Entity_Manager::get_all_entities()
{
    return &entities;
}

std::vector<Entity *> Entity_Manager::get_entities_in_chunks(
    const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks)
{
    std::vector<Entity *> result;

    for (auto &[id, entity] : entities)
    {
        if (!entity->is_active)
            continue;

        glm::ivec2 entity_chunk = entity->get_chunk_position(chunk_pixel_width, chunk_pixel_height);

        if (active_chunks.find(entity_chunk) != active_chunks.end())
        {
            result.push_back(entity.get());
        }
    }

    return result;
}

std::vector<Entity *> Entity_Manager::get_all_active_entities()
{
    std::vector<Entity *> result;

    for (auto &[id, entity] : entities)
    {
        if (entity->is_active)
        {
            result.push_back(entity.get());
        }
    }

    return result;
}

int Entity_Manager::get_entity_count() const
{
    return static_cast<int>(entities.size());
}

int Entity_Manager::get_enemy_count() const
{
    int count = 0;
    for (const auto &[id, entity] : entities)
    {
        if (entity->type == Entity_Type::ENEMY && entity->is_active)
        {
            count++;
        }
    }
    return count;
}

int Entity_Manager::get_enemy_count_in_chunks(
    const std::unordered_set<glm::ivec2, Chunk_Coords_to_Hash> &active_chunks) const
{
    int count = 0;

    for (const auto &[id, entity] : entities)
    {
        if (!entity->is_active || entity->type != Entity_Type::ENEMY)
            continue;

        glm::ivec2 entity_chunk = entity->get_chunk_position(chunk_pixel_width, chunk_pixel_height);
        if (active_chunks.find(entity_chunk) != active_chunks.end())
        {
            ++count;
        }
    }

    return count;
}

int Entity_Manager::get_devushki_count() const
{
    int count = 0;
    for (const auto &[id, entity] : entities)
    {
        if (entity->type == Entity_Type::DEVUSHKI && entity->is_active)
        {
            count++;
        }
    }
    return count;
}

int Entity_Manager::get_boss_count() const
{
    int count = 0;
    for (const auto &[id, entity] : entities)
    {
        if (entity->type == Entity_Type::BOSS && entity->is_active)
        {
            count++;
        }
    }
    return count;
}

int Entity_Manager::get_collected_gold_coins() const
{
    return collected_gold_coins;
}

int Entity_Manager::get_collected_silver_coins() const
{
    return collected_silver_coins;
}

int Entity_Manager::get_player_ammo() const
{
    return player_ammo;
}

bool Entity_Manager::has_compass() const
{
    return player_has_compass;
}

bool Entity_Manager::get_nearest_devushki_position(glm::ivec2 &out_position, float *out_distance) const
{
    if (!player)
        return false;

    const Entity *nearest = nullptr;
    float nearest_dist_sq = std::numeric_limits<float>::max();

    for (const auto &[id, entity] : entities)
    {
        if (!entity || !entity->is_active || !entity->get_is_alive())
            continue;
        if (entity->type != Entity_Type::DEVUSHKI)
            continue;

        const float dx = static_cast<float>(entity->coords.x - player->coords.x);
        const float dy = static_cast<float>(entity->coords.y - player->coords.y);
        const float dist_sq = dx * dx + dy * dy;

        if (dist_sq < nearest_dist_sq)
        {
            nearest = entity.get();
            nearest_dist_sq = dist_sq;
        }
    }

    if (!nearest)
        return false;

    out_position = nearest->coords;
    if (out_distance)
        *out_distance = std::sqrt(nearest_dist_sq);
    return true;
}

bool Entity_Manager::get_compass_target_position(glm::ivec2 &out_position, float *out_distance) const
{
    if (!player)
        return false;

    if (devushki_objective.objective_complete && devushki_objective.boss_spawned)
    {
        const Entity *nearest_boss = nullptr;
        float nearest_dist_sq = std::numeric_limits<float>::max();

        for (const auto &[id, entity] : entities)
        {
            if (!entity || !entity->is_active || !entity->get_is_alive())
                continue;
            if (entity->type != Entity_Type::BOSS)
                continue;

            const float dx = static_cast<float>(entity->coords.x - player->coords.x);
            const float dy = static_cast<float>(entity->coords.y - player->coords.y);
            const float dist_sq = dx * dx + dy * dy;

            if (dist_sq < nearest_dist_sq)
            {
                nearest_boss = entity.get();
                nearest_dist_sq = dist_sq;
            }
        }

        if (nearest_boss)
        {
            out_position = nearest_boss->coords;
            if (out_distance)
                *out_distance = std::sqrt(nearest_dist_sq);
            return true;
        }
    }

    return get_nearest_devushki_position(out_position, out_distance);
}

bool Entity_Manager::try_consume_ammo_for_shot()
{
    if (player_ammo <= 0)
        return false;

    --player_ammo;
    return true;
}

void Entity_Manager::add_player_ammo(int amount)
{
    if (amount <= 0)
        return;

    player_ammo += amount;
}

bool Entity_Manager::is_player_near_store() const
{
    return get_nearest_store_offer() != nullptr;
}

const Store_Offer *Entity_Manager::get_nearest_store_offer() const
{
    if (!player)
        return nullptr;

    const float interact_range_sq = STORE_INTERACT_RANGE_PX * STORE_INTERACT_RANGE_PX;
    const Store_Offer *nearest = nullptr;
    float nearest_dist_sq = std::numeric_limits<float>::max();

    for (const auto &[key, offer] : store_offers_by_structure)
    {
        if (offer.purchased)
            continue;

        const float dx = static_cast<float>(player->coords.x - offer.display_world_pos.x);
        const float dy = static_cast<float>(player->coords.y - offer.display_world_pos.y);
        const float dist_sq = dx * dx + dy * dy;
        if (dist_sq > interact_range_sq)
            continue;

        if (dist_sq < nearest_dist_sq)
        {
            nearest = &offer;
            nearest_dist_sq = dist_sq;
        }
    }

    return nearest;
}

std::vector<Store_Offer> Entity_Manager::get_active_store_offers() const
{
    std::vector<Store_Offer> result;
    result.reserve(store_offers_by_structure.size());

    for (const auto &[key, offer] : store_offers_by_structure)
    {
        if (offer.purchased)
            continue;
        result.push_back(offer);
    }

    return result;
}

bool Entity_Manager::try_buy_store_item()
{
    if (!player)
        return false;

    const Store_Offer *offer_ptr = get_nearest_store_offer();
    if (!offer_ptr)
        return false;

    auto it = store_offers_by_structure.find(offer_ptr->structure_hash);
    if (it == store_offers_by_structure.end() || it->second.purchased)
        return false;

    Store_Offer &offer = it->second;
    const int price_silver_total = price_to_silver(offer.price_gold, offer.price_silver);
    const int current_total = get_total_currency_in_silver();
    if (current_total < price_silver_total)
        return false;

    auto grant_wand = [&](Wand wand) -> bool
    {
        Hotbar &hotbar = player->get_hotbar();
        for (int slot = 0; slot < Hotbar::size(); ++slot)
        {
            if (hotbar.get_wand(slot).is_empty())
            {
                hotbar.set_wand(slot, wand);
                return true;
            }
        }
        return false;
    };

    bool applied = false;
    switch (offer.type)
    {
    case Store_Offer_Type::HEAL:
        if (player->healthpoints < player->max_healthpoints)
        {
            player->heal(static_cast<float>(STORE_HEAL_AMOUNT));
            applied = true;
        }
        break;
    case Store_Offer_Type::AMMO:
        add_player_ammo(AMMO_PURCHASE_AMOUNT);
        applied = true;
        break;
    case Store_Offer_Type::WAND_FIRE:
        applied = grant_wand(Wand::create_fire_wand());
        break;
    case Store_Offer_Type::WAND_WOOD:
        applied = grant_wand(Wand::create_wood_wand());
        break;
    case Store_Offer_Type::WAND_EMPTY:
    {
        Wand empty_wand = Wand::create_delete_wand();
        empty_wand.name = "Empty Wand";
        applied = grant_wand(empty_wand);
        break;
    }
    case Store_Offer_Type::COMPASS:
        if (!player_has_compass)
        {
            player_has_compass = true;
            applied = true;
        }
        break;
    }

    if (!applied)
        return false;

    const int new_total = current_total - price_silver_total;
    collected_gold_coins = new_total / SILVER_PER_GOLD;
    collected_silver_coins = new_total % SILVER_PER_GOLD;
    normalize_currency();
    offer.purchased = true;
    return true;
}

int Entity_Manager::get_store_heal_price_gold() const
{
    return STORE_HEAL_PRICE_GOLD;
}

int Entity_Manager::get_store_heal_amount() const
{
    return STORE_HEAL_AMOUNT;
}

bool Entity_Manager::has_entity(int id) const
{
    return entities.find(id) != entities.end();
}

// ==================== Devushki Objective System ====================

void Entity_Manager::spawn_devushki_objective(int count, float spread_radius, const std::string &sprite_name)
{
    // Set up the objective - devushki will be spawned as structures are placed
    devushki_objective.total_to_collect = count;
    devushki_objective.collected = 0;
    devushki_objective.objective_active = true;
    devushki_objective.objective_complete = false;
    devushki_objective.boss_spawned = false;

    // Store sprite name for deferred spawning
    devushki_sprite_name = sprite_name;

    // Clear spawned positions to start fresh
    spawned_devushki_positions.clear();
}

void Entity_Manager::spawn_boss_for_completed_objective()
{
    if (devushki_objective.boss_spawned || !player)
        return;

    if (get_boss_count() > 0)
    {
        devushki_objective.boss_spawned = true;
        return;
    }

    // Keep boss spawn in a fair annulus around the player (not too close, not too far).
    const float min_distance = std::max(260.0f, spawn_config.min_spawn_distance + 40.0f);
    const float max_distance = std::max(min_distance + 140.0f, spawn_config.max_spawn_distance + 80.0f);
    const float min_distance_sq = min_distance * min_distance;
    const float max_distance_sq = max_distance * max_distance;

    auto is_within_spawn_radius = [&](const glm::ivec2 &pos) -> bool
    {
        const float dx = static_cast<float>(pos.x - player->coords.x);
        const float dy = static_cast<float>(pos.y - player->coords.y);
        const float dist_sq = dx * dx + dy * dy;
        return dist_sq >= min_distance_sq && dist_sq <= max_distance_sq;
    };

    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * 3.14159265f);
    std::uniform_real_distribution<float> dist_dist(min_distance, max_distance);

    Boss *boss = nullptr;

    // Try several random candidates and only accept those that are both safe and inside the annulus.
    for (int i = 0; i < 80 && !boss; ++i)
    {
        const float angle = angle_dist(rng);
        const float distance = dist_dist(rng);

        glm::ivec2 candidate(
            player->coords.x + static_cast<int>(std::cos(angle) * distance),
            player->coords.y + static_cast<int>(std::sin(angle) * distance));

        if (world)
        {
            auto probe = std::make_unique<Boss>();
            probe->set_world(world);
            candidate = find_safe_spawn_position(probe.get(), world, rng, candidate, true, 700);
        }

        if (!is_within_spawn_radius(candidate))
            continue;

        Boss *spawned = create_boss(candidate, "big_boss");
        if (!spawned)
            continue;

        if (!is_within_spawn_radius(spawned->coords))
        {
            remove_entity(spawned->get_id());
            continue;
        }

        boss = spawned;
    }

    if (!boss)
        return;

    boss->name = "Boss of Devushki";

    const float difficulty_scale = std::max(1.0f, spawn_config.difficulty_multiplier);
    boss->max_healthpoints *= difficulty_scale;
    boss->healthpoints = boss->max_healthpoints;
    boss->set_attack_damage(boss->get_attack_damage() * difficulty_scale);

    devushki_objective.boss_spawned = true;
}

void Entity_Manager::check_and_spawn_devushki_on_structures()
{
    if (!world || !devushki_objective.objective_active)
        return;

    // Get all placed devushki_column structures from the world
    const auto &placed_structures = world->get_structure_spawner().get_placed_structures();

    // Get devushki_column structure for its dimensions
    Structure *devushki_col = world->get_image_structure("devushki_column");
    if (!devushki_col)
        return;

    int struct_width_px = static_cast<int>(devushki_col->get_width() * Globals::PARTICLE_SIZE);

    int current_devushki_count = get_devushki_count();

    for (const auto &ps : placed_structures)
    {
        if (ps.name != "devushki_column")
            continue;

        // Check if we've already spawned on this position
        // Create a simple hash from position
        int pos_hash = ps.position.x * 73856093 ^ ps.position.y * 19349663;

        if (spawned_devushki_positions.count(pos_hash) > 0)
            continue; // Already spawned here

        // Check if we've reached the objective count
        if (current_devushki_count >= devushki_objective.total_to_collect)
            break;

        // Calculate center-top position of the column
        glm::ivec2 spawn_pos;
        spawn_pos.x = ps.position.x + struct_width_px / 2;
        spawn_pos.y = ps.position.y; // top of structure

        Devushki *d = create_devushki(spawn_pos, devushki_sprite_name);
        if (d)
        {
            const glm::ivec2 anchored_pos = find_devushki_column_anchor_position(
                d,
                world,
                ps.position,
                struct_width_px);

            d->set_position(anchored_pos);
            d->set_home_position(anchored_pos);
            d->set_velocity(0.0f, 0.0f);

            d->name = "Devushki #" + std::to_string(current_devushki_count + 1);
            spawned_devushki_positions.insert(pos_hash);
            current_devushki_count++;
        }
    }
}

void Entity_Manager::check_devushki_collection()
{
    if (!devushki_objective.objective_active || devushki_objective.objective_complete)
        return;
    if (!player)
        return;

    float collect_range_sq = devushki_objective.collect_range * devushki_objective.collect_range;

    for (auto it = entities.begin(); it != entities.end();)
    {
        Entity *entity = it->second.get();
        if (entity->type == Entity_Type::DEVUSHKI && entity->is_active)
        {
            float dx = static_cast<float>(player->coords.x - entity->coords.x);
            float dy = static_cast<float>(player->coords.y - entity->coords.y);
            float dist_sq = dx * dx + dy * dy;

            if (dist_sq <= collect_range_sq)
            {
                // Collected!
                devushki_objective.collected++;
                it = entities.erase(it);

                // Check if objective complete
                if (devushki_objective.collected >= devushki_objective.total_to_collect)
                {
                    devushki_objective.objective_complete = true;
                }
                continue;
            }
        }
        ++it;
    }
}

void Entity_Manager::set_devushki_objective_count(int count)
{
    devushki_objective.total_to_collect = count;
}

void Entity_Manager::set_devushki_collect_range(float range)
{
    devushki_objective.collect_range = range;
}

DevushkiObjective &Entity_Manager::get_devushki_objective()
{
    return devushki_objective;
}

bool Entity_Manager::is_objective_complete() const
{
    return devushki_objective.objective_complete;
}

// ==================== Sprite Management ====================

void Entity_Manager::register_sprite(const std::string &name, const std::string &sprite_path)
{
    // Default format: 128x32 sprite sheet, 4 frames of 32x32
    // Frame 0: facing left, Frame 1: facing right, Frame 2: jumping, Frame 3: hurt
    register_sprite(name, sprite_path, 128, 32, 32, 32, 4);
}

void Entity_Manager::register_sprite(const std::string &name, const std::string &sprite_path,
                                     int sheet_width, int sheet_height, int frame_width, int frame_height, int frame_count)
{
    SpriteConfig config;
    config.path = sprite_path;
    config.sheet_width = sheet_width;
    config.sheet_height = sheet_height;
    config.frame_width = frame_width;
    config.frame_height = frame_height;
    config.frame_count = frame_count;
    config.is_valid = true;

    sprite_registry[name] = config;
}

void Entity_Manager::apply_sprite(Entity *entity, const std::string &name)
{
    if (!entity || name.empty())
        return;

    auto it = sprite_registry.find(name);
    if (it == sprite_registry.end() || !it->second.is_valid)
        return;

    const SpriteConfig &config = it->second;
    entity->setup_sprite_sheet(config.path, config.sheet_width, config.sheet_height,
                               config.frame_width, config.frame_height, config.frame_count);
}

bool Entity_Manager::has_sprite(const std::string &name) const
{
    auto it = sprite_registry.find(name);
    return it != sprite_registry.end() && it->second.is_valid;
}

const SpriteConfig *Entity_Manager::get_sprite_config(const std::string &name) const
{
    auto it = sprite_registry.find(name);
    if (it != sprite_registry.end())
    {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> Entity_Manager::get_all_sprite_names() const
{
    std::vector<std::string> names;
    names.reserve(sprite_registry.size());
    for (const auto &[name, config] : sprite_registry)
    {
        if (config.is_valid)
        {
            names.push_back(name);
        }
    }
    return names;
}