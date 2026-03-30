#pragma once

#include <string>
#include <glm/glm.hpp>
#include "engine/particle/particle.hpp"

enum class Wand_Type
{
    NONE,
    SAND_WAND,
    WATER_WAND,
    WOOD_WAND,
    FIRE_WAND,
    STONE_WAND,
    DELETE_WAND,
    SMOKE_WAND,
    URANIUM_WAND,
    GUN_WAND,
    ICE_WAND,
    WATER_VAPOR_WAND,
    COMPASS_WAND
};

struct Wand
{
    Wand_Type type = Wand_Type::NONE;
    std::string name = "Empty";
    glm::vec4 color = {0.5f, 0.5f, 0.5f, 1.0f}; // Wand line color
    Particle_Type particle_type = Particle_Type::EMPTY;
    int brush_size = 1;    // How many particles to place at once
    float cooldown = 0.0f; // Time between uses
    float last_use_time = 0.0f;

    bool is_empty() const { return type == Wand_Type::NONE; }

    // Factory methods for creating wands
    static Wand create_sand_wand()
    {
        Wand w;
        w.type = Wand_Type::SAND_WAND;
        w.name = "Sand Wand";
        w.color = {0.96f, 0.84f, 0.55f, 1.0f}; // Sandy color
        w.particle_type = Particle_Type::SAND;
        w.brush_size = 5;
        return w;
    }

    static Wand create_water_wand()
    {
        Wand w;
        w.type = Wand_Type::WATER_WAND;
        w.name = "Water Wand";
        w.color = {0.2f, 0.5f, 1.0f, 1.0f}; // Blue water color
        w.particle_type = Particle_Type::WATER;
        w.brush_size = 5;
        return w;
    }

    static Wand create_ice_wand()
    {
        Wand w;
        w.type = Wand_Type::ICE_WAND;
        w.name = "Ice Wand";
        w.color = {0.78f, 0.93f, 1.0f, 1.0f};
        w.particle_type = Particle_Type::ICE;
        w.brush_size = 5;
        return w;
    }

    static Wand create_water_vapor_wand()
    {
        Wand w;
        w.type = Wand_Type::WATER_VAPOR_WAND;
        w.name = "Water Vapor Wand";
        w.color = {0.88f, 0.92f, 0.98f, 0.85f};
        w.particle_type = Particle_Type::WATER_VAPOR;
        w.brush_size = 5;
        return w;
    }

    static Wand create_stone_wand()
    {
        Wand w;
        w.type = Wand_Type::STONE_WAND;
        w.name = "Stone Wand";
        w.color = {0.5f, 0.5f, 0.5f, 1.0f}; // Gray stone color
        w.particle_type = Particle_Type::STONE;
        w.brush_size = 5;
        return w;
    }

    static Wand create_wood_wand()
    {
        Wand w;
        w.type = Wand_Type::WOOD_WAND;
        w.name = "Wood Wand";
        w.color = {0.55f, 0.37f, 0.24f, 1.0f}; // Brown wood color
        w.particle_type = Particle_Type::WOOD;
        w.brush_size = 5;
        return w;
    }

    static Wand create_fire_wand()
    {
        Wand w;
        w.type = Wand_Type::FIRE_WAND;
        w.name = "Fire Wand";
        w.color = {1.0f, 0.45f, 0.12f, 1.0f}; // Orange flame color
        w.particle_type = Particle_Type::FIRE;
        w.brush_size = 3;
        w.cooldown = 0.02f;
        return w;
    }

    static Wand create_delete_wand()
    {
        Wand w;
        w.type = Wand_Type::DELETE_WAND;
        w.name = "Delete Wand";
        w.color = {1.0f, 0.2f, 0.2f, 1.0f}; // Red delete color
        w.particle_type = Particle_Type::EMPTY;
        w.brush_size = 5;
        return w;
    }

    static Wand create_smoke_wand()
    {
        Wand w;
        w.type = Wand_Type::SMOKE_WAND;
        w.name = "Smoke Wand";
        w.color = {0.6f, 0.6f, 0.7f, 0.8f}; // Gray smoke color
        w.particle_type = Particle_Type::SMOKE;
        w.brush_size = 5;
        return w;
    }

    static Wand create_uranium_wand()
    {
        Wand w;
        w.type = Wand_Type::URANIUM_WAND;
        w.name = "Uranium Wand";
        w.color = {0.2f, 1.0f, 0.2f, 1.0f}; // Green uranium color
        w.particle_type = Particle_Type::URANIUM;
        w.brush_size = 5;
        return w;
    }

    static Wand create_gun_wand()
    {
        Wand w;
        w.type = Wand_Type::GUN_WAND;
        w.name = "Gun";
        w.color = {1.0f, 0.85f, 0.25f, 1.0f};
        w.particle_type = Particle_Type::STONE;
        w.brush_size = 1;
        w.cooldown = 0.08f;
        return w;
    }

    static Wand create_compass_wand()
    {
        Wand w;
        w.type = Wand_Type::COMPASS_WAND;
        w.name = "Compass";
        w.color = {1.0f, 0.9f, 0.25f, 1.0f};
        w.particle_type = Particle_Type::SAND;
        w.brush_size = 1;
        w.cooldown = 0.25f;
        return w;
    }
};

class Hotbar
{
private:
    static constexpr int HOTBAR_SIZE = 9;
    Wand slots[HOTBAR_SIZE];
    int selected_slot = 0;

public:
    Hotbar()
    {
        // Initialize default hotbar with wands
        slots[0] = Wand::create_gun_wand();
        slots[1] = Wand::create_stone_wand();
        slots[2] = Wand::create_ice_wand();
        slots[3] = Wand::create_water_vapor_wand();
        slots[4] = Wand::create_compass_wand();
        // slots[6] = Wand::create_sand_wand();
        // slots[2] = Wand::create_water_wand();
        // slots[3] = Wand::create_delete_wand();
        // slots[4] = Wand::create_smoke_wand();
        // slots[5] = Wand::create_uranium_wand();
        slots[7] = Wand::create_wood_wand();
        slots[8] = Wand::create_fire_wand();
    }

    void select_slot(int slot)
    {
        if (slot >= 0 && slot < HOTBAR_SIZE)
        {
            selected_slot = slot;
        }
    }

    int get_selected_slot() const { return selected_slot; }

    Wand &get_selected_wand() { return slots[selected_slot]; }
    const Wand &get_selected_wand() const { return slots[selected_slot]; }

    Wand &get_wand(int slot)
    {
        if (slot >= 0 && slot < HOTBAR_SIZE)
            return slots[slot];
        return slots[0];
    }

    void set_wand(int slot, const Wand &wand)
    {
        if (slot >= 0 && slot < HOTBAR_SIZE)
        {
            slots[slot] = wand;
        }
    }

    static constexpr int size() { return HOTBAR_SIZE; }
};
