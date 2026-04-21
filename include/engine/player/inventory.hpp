#pragma once

// File purpose: Defines the player inventory container and item operations.
#include <array>
#include "engine/player/item.hpp"

// Stores stackable item counts.
class Inventory
{
private:
    std::array<Item, 50> inventory;

public:
    // Returns inventory.
    std::array<Item, 50> *get_inventory();
    // Sets inventory.
    void set_inventory();

    // Removes item.
    void remove_item(const int inventory_index);
    // Adds item.
    void add_item(const Item &item, const int index);
    // Swaps items.
    void swap_items(const int inventory_index, const int other_index);
};
