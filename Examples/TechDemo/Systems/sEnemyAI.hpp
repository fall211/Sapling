//
//  sEnemyAI.hpp
//  Sapling TechDemo
//

#pragma once

#include "Utility/Debug.hpp"
#include "ECS/EntityManager.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "Core/AudioEngine.hpp"

#include "EnemyAI.hpp"
#include "PlayerController.hpp"

#include "glm/glm.hpp"
#include "glm/geometric.hpp"

#include <cmath>

namespace System
{
    struct EnemyAIBounds
    {
        float minX = 32.0f;
        float minY = 32.0f;
        float maxX = 608.0f;
        float maxY = 448.0f;
    };

    inline void EnemyMovement(std::shared_ptr<EntityManager>& entityManager, float dt, const EnemyAIBounds& bounds = {})
    {
        auto& enemies = entityManager->getEntities("enemy");
        auto& players = entityManager->getEntities("player");

        // Get player position for chase enemies
        glm::vec2 playerPos = glm::vec2(0.0f);
        bool hasPlayer = false;
        if (!players.empty() && players.front()->hasComponent<Comp::Transform>())
        {
            playerPos = glm::vec2(players.front()->getComponent<Comp::Transform>().position);
            hasPlayer = true;
        }

        for (auto& e : enemies)
        {
            if (!e->hasComponent<Comp::EnemyAI>() || !e->hasComponent<Comp::Transform>())
                continue;

            auto& ai = e->getComponent<Comp::EnemyAI>();
            auto& transform = e->getComponent<Comp::Transform>();

            if (!ai.active)
                continue;

            switch (ai.patrolType)
            {
                case Comp::EnemyAI::PatrolType::HORIZONTAL:
                case Comp::EnemyAI::PatrolType::VERTICAL:
                {
                    // Move in the current direction
                    float moveAmount = ai.moveSpeed * dt;
                    transform.position += glm::vec3(ai.direction * moveAmount, 0.0f);
                    ai.distanceTraveled += moveAmount;

                    // Reverse direction when patrol distance is reached
                    if (ai.distanceTraveled >= ai.patrolDistance)
                    {
                        ai.reverseDirection();
                    }

                    // Also reverse if hitting arena boundaries
                    float halfSize = 8.0f;
                    if (transform.position.x <= bounds.minX + halfSize ||
                        transform.position.x >= bounds.maxX - halfSize)
                    {
                        ai.direction.x = -ai.direction.x;
                        ai.distanceTraveled = 0.0f;
                        transform.position.x = std::max(bounds.minX + halfSize,
                            std::min(bounds.maxX - halfSize, transform.position.x));
                    }
                    if (transform.position.y <= bounds.minY + halfSize ||
                        transform.position.y >= bounds.maxY - halfSize)
                    {
                        ai.direction.y = -ai.direction.y;
                        ai.distanceTraveled = 0.0f;
                        transform.position.y = std::max(bounds.minY + halfSize,
                            std::min(bounds.maxY - halfSize, transform.position.y));
                    }

                    break;
                }

                case Comp::EnemyAI::PatrolType::CHASE:
                {
                    if (!hasPlayer)
                        break;

                    glm::vec2 toPlayer = playerPos - glm::vec2(transform.position);
                    float distToPlayer = glm::length(toPlayer);

                    // Only chase if player is within chase range
                    if (distToPlayer > 0.01f && distToPlayer <= ai.chaseRange)
                    {
                        ai.direction = glm::normalize(toPlayer);
                        float moveAmount = ai.moveSpeed * dt;
                        transform.position += glm::vec3(ai.direction * moveAmount, 0.0f);
                    }
                    else if (distToPlayer > ai.chaseRange)
                    {
                        // Idle - slight wander
                        ai.direction = glm::vec2(0.0f);
                    }

                    // Clamp to bounds
                    float halfSize = 8.0f;
                    transform.position.x = std::max(bounds.minX + halfSize,
                        std::min(bounds.maxX - halfSize, transform.position.x));
                    transform.position.y = std::max(bounds.minY + halfSize,
                        std::min(bounds.maxY - halfSize, transform.position.y));

                    break;
                }
            }

            // Flip sprite based on horizontal direction
            if (e->hasComponent<Comp::Sprite>())
            {
                auto& sprite = e->getComponent<Comp::Sprite>();
                if (ai.direction.x < -0.1f)
                {
                    sprite.flipX(true);
                }
                else if (ai.direction.x > 0.1f)
                {
                    sprite.flipX(false);
                }
            }
        }
    }

    // Damage cooldown tracker (simple static approach for the tech demo)
    namespace DamageState
    {
        inline float damageCooldown = 0.0f;
        inline const float COOLDOWN_TIME = 1.0f;

        inline void Reset()
        {
            damageCooldown = 0.0f;
        }
    }

    inline bool CheckEnemyPlayerCollision(std::shared_ptr<EntityManager>& entityManager, float dt)
    {
        // Update cooldown timer
        if (DamageState::damageCooldown > 0.0f)
        {
            DamageState::damageCooldown -= dt;
            return false;
        }

        auto& players = entityManager->getEntities("player");
        auto& enemies = entityManager->getEntities("enemy");

        if (players.empty() || enemies.empty())
            return false;

        for (auto& player : players)
        {
            if (!player->hasComponent<Comp::Transform>() || !player->hasComponent<Comp::PlayerController>())
                continue;

            auto& playerTransform = player->getComponent<Comp::Transform>();
            auto& controller = player->getComponent<Comp::PlayerController>();
            glm::vec2 playerPos = glm::vec2(playerTransform.position);

            for (auto& enemy : enemies)
            {
                if (!enemy->hasComponent<Comp::Transform>() || !enemy->hasComponent<Comp::EnemyAI>())
                    continue;

                auto& enemyAI = enemy->getComponent<Comp::EnemyAI>();

                if (!enemyAI.active)
                    continue;

                auto& enemyTransform = enemy->getComponent<Comp::Transform>();
                glm::vec2 enemyPos = glm::vec2(enemyTransform.position);

                float dist = glm::distance(playerPos, enemyPos);
                if (dist < enemyAI.damageRadius)
                {
                    // Player takes damage
                    controller.health--;
                    DamageState::damageCooldown = DamageState::COOLDOWN_TIME;

                    // Play hurt sound
                    AudioEngine::playSound("hurt");

                    // Flash the player sprite red
                    if (player->hasComponent<Comp::Sprite>())
                    {
                        player->getComponent<Comp::Sprite>().setColorOverride(
                            glm::vec4(1.0f, 0.2f, 0.2f, 0.8f), 0.3f
                        );
                    }

                    // Knock the player back
                    if (dist > 0.01f)
                    {
                        glm::vec2 knockback = glm::normalize(playerPos - enemyPos) * 40.0f;
                        playerTransform.position += glm::vec3(knockback, 0.0f);
                    }

                    Debug::log("Player hit! Health: " + std::to_string(controller.health));
                    return true;
                }
            }
        }

        return false;
    }
}