#pragma once

#include <array>
#include "engine/player/item.hpp"

class Inventory
{
private:
    std::array<Item, 50> inventory;

public:
    std::array<Item, 50> *get_inventory();
    void set_inventory();

    void remove_item(const int inventory_index);
    void add_item(const Item &item, const int index);
    void swap_items(const int inventory_index, const int other_index);
};
