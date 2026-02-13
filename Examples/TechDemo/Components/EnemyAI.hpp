//
//  EnemyAI.hpp
//  Sapling TechDemo
//

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "Utility/Debug.hpp"

#include "glm/glm.hpp"

#include <cstdlib>
#include <utility>

namespace Comp
{
    struct EnemyAI : public Component
    {
        enum class PatrolType : uint8_t
        {
            HORIZONTAL,
            VERTICAL,
            CHASE
        };

        PatrolType patrolType = PatrolType::HORIZONTAL;
        glm::vec2 direction = glm::vec2(1.0f, 0.0f);
        float moveSpeed = 50.0f;
        float patrolDistance = 80.0f;
        float distanceTraveled = 0.0f;
        float damageRadius = 14.0f;
        float chaseRange = 100.0f;
        bool active = true;

        explicit EnemyAI(Inst inst)
            : Component(std::move(inst))
        {}

        explicit EnemyAI(Inst inst, PatrolType type, float speed)
            : Component(std::move(inst)), patrolType(type), moveSpeed(speed)
        {
            if (type == PatrolType::VERTICAL)
            {
                direction = glm::vec2(0.0f, 1.0f);
            }
            else if (type == PatrolType::CHASE)
            {
                direction = glm::vec2(0.0f, 0.0f);
            }
        }

        explicit EnemyAI(Inst inst, PatrolType type, float speed, float distance)
            : Component(std::move(inst)), patrolType(type), moveSpeed(speed), patrolDistance(distance)
        {
            if (type == PatrolType::VERTICAL)
            {
                direction = glm::vec2(0.0f, 1.0f);
            }
            else if (type == PatrolType::CHASE)
            {
                direction = glm::vec2(0.0f, 0.0f);
            }
        }

        void reverseDirection()
        {
            direction = -direction;
            distanceTraveled = 0.0f;
        }

        void OnAddToEntity() override
        {
            inst->requestAddTag("enemy");
        }

        void OnRemoveFromEntity() override
        {
            inst->requestRemoveTag("enemy");
        }
    };
}