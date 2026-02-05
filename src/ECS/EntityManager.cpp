//
//  EntityManager.cpp
//  SaplingEngine
//

#include "ECS/EntityManager.hpp"

EntityManager::EntityManager()
{
    m_entities = EntityList();
    m_entitiesToAdd = EntityList();
    m_entityMap = EntityMap();
    m_spatialGrid = SpatialGrid(24.0f);
};

void EntityManager::update()
{
    // add new entities
    for (const auto& e : m_entitiesToAdd)
    {
        m_entities.push_back(e);
    }
    m_entitiesToAdd.clear();

    // collect all inactive entities
    std::vector<std::shared_ptr<Entity>> entitiesToDestroy;
    for (const auto& e: m_entities)
    {
        if (!e->isActive())
        {
            entitiesToDestroy.push_back(e);
        }
    }

    for (const auto& e: entitiesToDestroy)
    {
        destroyEntity(e);
    }
}

auto EntityManager::addEntity(const TagList& tags) -> std::shared_ptr<Entity>
{
    auto entity = std::shared_ptr<Entity>(new Entity(tags, m_idCounter++, shared_from_this()));
    m_entitiesToAdd.push_back(entity);
    for (const auto& tag : tags)
    {
        m_entityMap[tag].push_back(entity);
    }
    return entity;
}

auto EntityManager::getEntities() -> EntityList&
{    
    return m_entities;
}

auto EntityManager::getEntities(const std::string& tag) -> EntityList&
{
    return m_entityMap[tag];
}

void EntityManager::addTagToEntity(const std::shared_ptr<Entity>& entity, const std::string& tag)
{
    entity->addTag(tag);
    m_entityMap[tag].push_back(entity);
}

void EntityManager::removeTagFromEntity(const std::shared_ptr<Entity>& entity, const std::string& tag)
{
    entity->removeTag(tag);
    
    // Get the entity list with the given tag
    auto& entityListWithTag = m_entityMap[tag];
    
    // Use std::remove_if with a comparison that checks entity IDs
    entityListWithTag.erase(
        std::remove_if(
            entityListWithTag.begin(), 
            entityListWithTag.end(),
            [&entity](const std::shared_ptr<Entity>& e) {
                // Compare by entity ID or some unique identifier
                return e.get() == entity.get(); // Compare the raw pointers
            }
        ),
        entityListWithTag.end()
    );
}

void EntityManager::destroyEntity(const std::shared_ptr<Entity>& entity)
{
    for (const auto& tag : entity->getTags())
    {
        auto& entityListWithTag = m_entityMap[tag];
        entityListWithTag.erase(std::remove(entityListWithTag.begin(), entityListWithTag.end(), entity), entityListWithTag.end());
    }
    m_spatialGrid.removeEntity(entity);

    m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
}

void EntityManager::clear()
{
    m_spatialGrid.clear();
    
    m_entities.clear();
    m_entitiesToAdd.clear();
    
    for (auto& [tag, entityList] : m_entityMap)
    {
        entityList.clear();
    }
    m_entityMap.clear();
    
    m_idCounter = 0;
}

SpatialGrid& EntityManager::getSpatialGrid()
{
    return m_spatialGrid;
}

auto EntityManager::getEntitiesInRange(const glm::vec2 &center, float range) -> EntityList
{
    return m_spatialGrid.getEntitiesInRange(center, range);
}

auto EntityManager::getEntitiesInRange(const std::string& tag, const glm::vec2 &center, float range) -> EntityList
{
    auto entitiesInRange = m_spatialGrid.getEntitiesInRange(center, range);
    EntityList entitiesWithTagInRange;
    for (const auto& entity : entitiesInRange)
    {
        if (entity->hasTag(tag))
        {
            entitiesWithTagInRange.push_back(entity);
        }
    }
    return entitiesWithTagInRange;
}

auto EntityManager::getEntitiesInRange(const std::string& tag, EntityList& entitiesInRange) -> EntityList
{
    auto entitiesWithTagInRange = entitiesInRange;
    for (const auto& entity : entitiesInRange)
    {
        if (!entity->hasTag(tag))
        {
            entitiesWithTagInRange.erase(std::remove(entitiesWithTagInRange.begin(), entitiesWithTagInRange.end(), entity), entitiesWithTagInRange.end());
        }
    }
    return entitiesWithTagInRange;
}
