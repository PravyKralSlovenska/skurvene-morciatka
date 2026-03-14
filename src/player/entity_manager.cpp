#include "engine/player/entity_manager.hpp"

#include <cmath>
#include "engine/player/entity.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "others/GLOBALS.hpp"

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

void Entity_Manager::ensure_player_valid_position()
{
    if (!player || !world)
        return;

    glm::ivec2 valid_pos = player->find_valid_spawn_position(player->coords);
    player->set_position(valid_pos);
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

    // Find valid spawn position that fits the enemy's hitbox
    glm::ivec2 valid_pos = enemy->find_valid_spawn_position(position);
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

    // Find valid spawn position that fits the devushki's hitbox
    glm::ivec2 valid_pos = devushki->find_valid_spawn_position(position);
    devushki->set_position(valid_pos);
    devushki->set_home_position(valid_pos);

    int id = devushki->get_id();
    Devushki *devushki_ptr = devushki.get();
    entities[id] = std::move(devushki);

    // Apply registered sprite if name provided
    if (!sprite_name.empty())
    {
        apply_sprite(devushki_ptr, sprite_name);
    }

    return devushki_ptr;
}

Devushki *Entity_Manager::create_devushki(int x, int y, const std::string &sprite_name)
{
    return create_devushki(glm::ivec2(x, y), sprite_name);
}

Boss *Entity_Manager::create_boss(const glm::ivec2 &position, const std::string &sprite_name)
{
    auto boss = std::make_unique<Boss>();

    // Set world reference first so collision checks work
    if (world)
    {
        boss->set_world(world);
    }

    // Find valid spawn position that fits the boss's hitbox
    glm::ivec2 valid_pos = boss->find_valid_spawn_position(position);
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

    // Update player physics (gravity, collision) - input is handled separately
    if (player)
    {
        player->update(delta_time);
    }

    // Update all entities (enemies, NPCs, etc.)
    for (auto &[id, entity] : entities)
    {
        update_entity(entity.get(), delta_time);
    }

    // Check devushki collection
    check_devushki_collection();

    // Remove dead entities
    remove_all_dead();
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

        // Check if we can spawn more enemies
        if (get_enemy_count() < spawn_config.max_enemies)
        {
            spawn_random_enemy("slime");
        }
    }
}

glm::ivec2 Entity_Manager::get_random_spawn_position()
{
    if (!player)
        return {0, 0};

    // Generate random angle
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * 3.14159265f);
    float angle = angle_dist(rng);

    // Generate random distance between min and max
    std::uniform_real_distribution<float> dist_dist(
        spawn_config.min_spawn_distance,
        spawn_config.max_spawn_distance);
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
    glm::ivec2 spawn_pos = get_random_spawn_position();

    // Create a temporary enemy to check hitbox size for valid position
    auto temp = std::make_unique<Enemy>();
    if (world)
    {
        temp->set_world(world);
    }
    glm::ivec2 valid_pos = temp->find_valid_spawn_position(spawn_pos);

    Enemy *enemy = create_enemy(valid_pos, sprite_name);

    if (enemy)
    {
        enemy->set_home_position(valid_pos);
        randomize_enemy_stats(enemy);
    }

    return enemy;
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
    spawn_config.min_spawn_distance = min_dist;
    spawn_config.max_spawn_distance = max_dist;
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
    
    // Store sprite name for deferred spawning
    devushki_sprite_name = sprite_name;
    
    // Clear spawned positions to start fresh
    spawned_devushki_positions.clear();
}

void Entity_Manager::check_and_spawn_devushki_on_structures()
{
    if (!world || !devushki_objective.objective_active)
        return;
    
    // Get all placed devushki_column structures from the world
    const auto& placed_structures = world->get_structure_spawner().get_placed_structures();
    
    // Get devushki_column structure for its dimensions
    Structure* devushki_col = world->get_image_structure("devushki_column");
    if (!devushki_col)
        return;
    
    int struct_width_px = static_cast<int>(devushki_col->get_width() * Globals::PARTICLE_SIZE);
    
    int current_devushki_count = get_devushki_count();
    
    for (const auto& ps : placed_structures)
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