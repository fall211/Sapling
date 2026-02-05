//
//  EntityManager.hpp
//  SaplingEngine
//

#pragma once

#include "Utility/SpatialGrid.hpp"

#include <string>
#include <map>
#include <memory>
#include <vector>

class Entity;

typedef std::vector<std::shared_ptr<Entity>> EntityList;
typedef std::map<std::string, EntityList> EntityMap;
typedef std::vector<std::string> TagList;

class EntityManager : public std::enable_shared_from_this<EntityManager>
{
    private:
        EntityList m_entities;
        EntityList m_entitiesToAdd;
        EntityMap m_entityMap;
        size_t m_idCounter = 0;
        SpatialGrid m_spatialGrid;
    
    public:
        EntityManager();
        
        /*
            * Adds new entities to the manager and deletes destroyed entities.
        */
        void update();
        
        template <typename T, typename... Args>
        auto instantiatePrefab(Args... args) -> std::shared_ptr<Entity>;
        
        /*
            * Adds a new entity to the manager with the given tags.
            * @param tags The tags to add to the entity
            * @return The new entity
        */
        auto addEntity(const TagList& tags) -> std::shared_ptr<Entity>;
        
        /*
            * Gets all entities in the manager
            * @return The list of entities
        */
        auto getEntities() -> EntityList&;
        
        /*
            * Gets all entities with the given tag
            * @param tag The tag to search for
            * @return The list of entities with the given tag
        */
        auto getEntities(const std::string& tag) -> EntityList&;
        
        /*
            * Gets all entities with the given component
            * @tparam T The component type
            * @return The list of entities with the given component
        */
        template <typename T>
        auto getEntitiesByComponent() -> EntityList&;
        
        /*
            * Adds a tag to the given entity
            * @param entity The entity to add the tag to
            * @param tag The tag to add
        */
        void addTagToEntity(const std::shared_ptr<Entity>& entity, const std::string& tag);
        
        /*
            * Removes a tag from the given entity
            * @param entity The entity to remove the tag from
            * @param tag The tag to remove
        */
        void removeTagFromEntity(const std::shared_ptr<Entity>& entity, const std::string& tag);
        
        /*
            * Destroys the given entity
            * @param entity The entity to destroy
        */
        void destroyEntity(const std::shared_ptr<Entity>& entity);
        
        /*
            * Clears all entities from the entity manager
        */
        void clear();
        
        /*
            * Gets the spatial grid used by the entity manager
            * @return The spatial grid
        */
        SpatialGrid& getSpatialGrid();
        
        
        /*
            * Gets all entities within a certain range of a given position
            * @param center The center of the range
            * @param range The range of the search
            * @return The list of entities within the range
        */
        EntityList getEntitiesInRange(const glm::vec2 &center, float range);
        
        /*
            * Gets all entities within a certain range of a given position
            * @param tag The tag of the entities to get
            * @param center The center of the range
            * @param range The range of the search
            * @return The list of entities within the range
        */
        EntityList getEntitiesInRange(const std::string& tag, const glm::vec2 &center, float range);
        
        /*
            * Gets all entities within a certain range of a given position
            * @param tag The tag of the entities to get
            * @param entitiesInRange The list of entities within the range
            * @return The list of entities within the range
        */
        EntityList getEntitiesInRange(const std::string& tag, EntityList& entitiesInRange);
};

#include "EntityManagerT.hpp"