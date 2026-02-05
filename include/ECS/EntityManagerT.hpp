//
//  EntityManagerT.hpp
//  SaplingEngine
//

#pragma once

#include "ECS/EntityManager.hpp"
#include "ECS/Entity.hpp"

template <typename T, typename... Args>
auto EntityManager::instantiatePrefab(Args... args) -> std::shared_ptr<Entity>
{
    auto entity = std::shared_ptr<Entity>(new Entity({"prefab"}, m_idCounter++, shared_from_this()));
    auto prefab = std::shared_ptr<T>(new T(entity, std::forward<Args>(args)...));
    m_entitiesToAdd.push_back(entity);
    m_spatialGrid.updateEntity(entity);
    return entity;
}

template <typename T>
auto EntityManager::getEntitiesByComponent() -> EntityList&
{
    static EntityList entities;
    entities.clear();
    for (const auto& e : m_entities){
        if (e && e->hasComponent<T>()){
            entities.push_back(e);
        }
    }
    return entities;
}