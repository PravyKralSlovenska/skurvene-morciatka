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
    void remove_all_dead();

public:
    Entity_Manager();
    ~Entity_Manager();

    void update();
    
    void create_entity();
    void remove_entity(const int id);
    
    Player *get_player();
    Entity* get_entity(const int id);
};
