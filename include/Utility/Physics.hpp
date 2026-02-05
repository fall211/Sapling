//
//  Physics.hpp
//  SaplingEngine, Twig Physics
//

#pragma once

#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "Renderer/Sprout.hpp"

#include "glm/glm.hpp"
#include "glm/geometric.hpp"

#include <memory>
#include <set>


class Entity;

class Physics2D 
{
    public:
    
        /*
            * Detects the overlap of the bounding boxes of the two entities e0 and e1.
            * @param e0 The first entity
            * @param e1 The second entity
            * @return The overlap vector
        */
        static auto bBoxCollision(const std::shared_ptr<Entity>& e0, const std::shared_ptr<Entity>& e1) -> glm::vec2;
        
        /*
            * Detects the overlap of the bounding circles of the two entities e0 and e1.
            * @param e0 The first entity
            * @param e1 The second entity
            * @return The overlap vector
        */
        static auto bCircleCollision(const std::shared_ptr<Entity>& e0, const std::shared_ptr<Entity>& e1) -> glm::vec2;
        
        /*
            * Detects the overlap of the bounding box of eBox and the bounding circle of eCircle.
            * @param eBox The entity with the bounding box
            * @param eCircle The entity with the bounding circle
            * @return The overlap vector
        */
        static auto bBoxCircleCollision(const std::shared_ptr<Entity>& eBox, const std::shared_ptr<Entity>& eCircle) -> glm::vec2;
        
        
        enum CollisionType
        {
            STATIC_STATIC,
            DYNAMIC_DYNAMIC,
            STATIC_DYNAMIC,
            DYNAMIC_STATIC,
            NONE
        };
        
        struct CollisionData
        {
            glm::vec2 overlap = glm::vec2(0.0f);
            glm::vec2 normal = glm::vec2(0.0f);
            CollisionType type = NONE;
            bool trigger = false;
            bool triggerEvent = false;
            
        };
        
        static CollisionData collisionData(const std::shared_ptr<Entity>& e0, const std::shared_ptr<Entity>& e1);
};

