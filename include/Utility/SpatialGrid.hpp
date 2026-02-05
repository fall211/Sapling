//
//
//
//


#pragma once

#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"

#include "glm/glm.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

class SpatialGrid {
private:
    float cellSize;
    float inverseCellSize = 1/cellSize;
    std::unordered_map<int, std::vector<std::shared_ptr<Entity>>> cells;
    std::unordered_map<size_t, std::vector<int>> entityCells;
    
    inline int hashCoords(int x, int y) const 
    {
        return x * 73856093 ^ y * 19349663;
    }
    inline int getCellIndex(float x, float y) const {
        int cellX = static_cast<int>(x * inverseCellSize);
        int cellY = static_cast<int>(y * inverseCellSize);
        return hashCoords(cellX, cellY);
    }
    
    std::vector<int> getOverlappingCells(const glm::vec2& position, const glm::vec2& size) const {
        std::vector<int> cellIndices;
        
        float minX = position.x - (size.x/2.0f);
        float minY = position.y - (size.y/2.0f);
        float maxX = position.x + (size.x/2.0f);
        float maxY = position.y + (size.y/2.0f);
        
        int startX = static_cast<int>(floor(minX * inverseCellSize));
        int startY = static_cast<int>(floor(minY * inverseCellSize));
        int endX = static_cast<int>(floor(maxX * inverseCellSize));
        int endY = static_cast<int>(floor(maxY * inverseCellSize));
        
        cellIndices.reserve((endX - startX + 1) * (endY - startY + 1));
        for (int y = startY; y <= endY; y++) {
            for (int x = startX; x <= endX; x++) {
                cellIndices.push_back((x & 0xFFFF) | ((y & 0xFFFF) << 16));
            }
        }
        
        return cellIndices;
    }

public:
    explicit SpatialGrid(float cellSize = 48.0f) : cellSize(cellSize) {}
    
    void clear() {
        cells.clear();
        entityCells.clear();
    }
    
    void updateEntity(const std::shared_ptr<Entity>& entity) {
        if (!entity->hasComponent<Comp::Transform>()) {
            return;
        }
        
        const auto& transform = entity->getComponent<Comp::Transform>();

        if (!entity->hasComponent<Comp::BBox>()) {
            // no bounding box component so we just add it to the grid at the position of the entity
            // remove from old cells
            auto entityId = entity->getId();
            auto it = entityCells.find(entityId);
            if (it != entityCells.end()) {
                for (int cellIdx : it->second) {
                    auto& cellEntities = cells[cellIdx];
                    cellEntities.erase(
                        std::remove_if(cellEntities.begin(), cellEntities.end(),
                            [entityId](const auto& e) { return e->getId() == entityId; }),
                        cellEntities.end()
                    );
                    if (cellEntities.empty()) {
                        cells.erase(cellIdx);
                    }
                }
            }
            
            // insert into new cell
            auto newCells = getOverlappingCells(transform.position, {1,1});
            entityCells[entityId] = newCells;
            
            for (int idx : newCells) {
                cells[idx].push_back(entity);
            }
            return;
        }
        
        const auto& bbox = entity->getComponent<Comp::BBox>();
        
        // remove from old cells
        auto entityId = entity->getId();
        auto it = entityCells.find(entityId);
        if (it != entityCells.end()) {
            for (int cellIdx : it->second) {
                auto& cellEntities = cells[cellIdx];
                cellEntities.erase(
                    std::remove_if(cellEntities.begin(), cellEntities.end(),
                        [entityId](const auto& e) { return e->getId() == entityId; }),
                    cellEntities.end()
                );
                if (cellEntities.empty()) {
                    cells.erase(cellIdx);
                }
            }
        }
        
        // insert into new cells
        glm::vec2 size(bbox.w * abs(transform.scale.x), bbox.h * abs(transform.scale.y));
        auto newCells = getOverlappingCells(transform.position, size);
        entityCells[entityId] = newCells;
        
        for (int idx : newCells) {
            cells[idx].push_back(entity);
        }
    }
    
    void removeEntity(const std::shared_ptr<Entity>& entity) {
        if (!entity->hasComponent<Comp::Transform>()) {
            return;
        }
        
        const auto& transform = entity->getComponent<Comp::Transform>();

        if (!entity->hasComponent<Comp::BBox>()) {

            auto indices = getOverlappingCells(transform.position, {1,1});
            
            for (int idx : indices) {
                auto it = cells.find(idx);
                if (it != cells.end()) {
                    it->second.erase(std::remove(it->second.begin(), it->second.end(), entity), it->second.end());
                }
            }
            
            entityCells.erase(entity->getId());
            return;
        }
        
        const auto& bbox = entity->getComponent<Comp::BBox>();
        
        glm::vec2 size(bbox.w * abs(transform.scale.x), bbox.h * abs(transform.scale.y));
        auto indices = getOverlappingCells(transform.position, size);
        
        for (int idx : indices) {
            auto it = cells.find(idx);
            if (it != cells.end()) {
                it->second.erase(std::remove(it->second.begin(), it->second.end(), entity), it->second.end());
            }
        }
        
        entityCells.erase(entity->getId());
    }
    
    std::vector<std::shared_ptr<Entity>> getPotentialCollisions(const std::shared_ptr<Entity>& entity) const {
        std::vector<std::shared_ptr<Entity>> result;
        
        if (!entity->hasComponent<Comp::Transform>() || !entity->hasComponent<Comp::BBox>()) {
            return result;
        }
        
        const auto& transform = entity->getComponent<Comp::Transform>();
        const auto& bbox = entity->getComponent<Comp::BBox>();
        
        glm::vec2 size(bbox.w * abs(transform.scale.x), bbox.h * abs(transform.scale.y));
        auto indices = getOverlappingCells(transform.position, size);
        
        std::unordered_map<size_t, std::shared_ptr<Entity>> uniqueEntities;
        uniqueEntities.reserve(16);
        
        for (int idx : indices) {
            auto it = cells.find(idx);
            if (it != cells.end()) {
                for (const auto& other : it->second) {
                    if (other->getId() != entity->getId()) {
                        uniqueEntities[other->getId()] = other;
                    }
                }
            }
        }
        
        result.reserve(uniqueEntities.size());
        for (const auto& pair : uniqueEntities) {
            result.push_back(pair.second);
        }
        
        return result;
    }
    
    std::vector<std::shared_ptr<Entity>> getEntitiesInRange(const glm::vec2& center, float range) const 
    {
        std::vector<std::shared_ptr<Entity>> result;

        auto indices = getOverlappingCells(center, {range, range});

        std::unordered_map<size_t, std::shared_ptr<Entity>> uniqueEntities;
        uniqueEntities.reserve(128);

 
        for (int idx : indices) {
            auto it = cells.find(idx);
            if (it != cells.end()) {
                for (const auto& other : it->second) {
                    uniqueEntities[other->getId()] = other;
                }
            }
        }

        result.reserve(uniqueEntities.size());
        for (const auto& pair : uniqueEntities) {
            result.push_back(pair.second);
        }

        return result;
    }
};