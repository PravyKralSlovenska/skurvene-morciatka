#include "engine/player/entity_manager.hpp"

#include "engine/player/entity.hpp"

void Entity_Manager::create_entity()
{
    auto entity = std::make_unique<Entity>(); 
}

void Entity_Manager::remove_entity(const int id)
{
    // najdi ju v unordered liste a vymaz ju
    // zavolaj desturktor
}

void Entity_Manager::update()
{
    // prejdi cez kazdu entitu
    // ak su v active chunku
    // tak ju updateni
    // inak nie
}

Player* Entity_Manager::get_player()
{
    return player.get();
}

Entity* Entity_Manager::get_entity(const int id)
{
    return entities[id].get();
}