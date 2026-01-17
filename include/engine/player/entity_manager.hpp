#pragma once

#include <unordered_map>
#include <memory>

// forward declarations
class Entity;
class Player;

class Entity_Manager
{
private:
    std::unique_ptr<Player> player;
    std::unordered_map<int, std::unique_ptr<Entity>> entities;

private:
    void create_entity();
    void remove_entity();

public:
    Entity_Manager();
    ~Entity_Manager();

    void update();
};
