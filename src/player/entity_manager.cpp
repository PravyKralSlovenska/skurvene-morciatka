#include "engine/player/entity_manager.hpp"

#include "engine/player/entity.hpp"

void Entity_Manager::create_entity()
{
    auto entity = std::make_unique<Entity>(); 
}

void Entity_Manager::remove_entity()
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