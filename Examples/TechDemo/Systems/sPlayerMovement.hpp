//
//  sPlayerMovement.hpp
//  Sapling TechDemo
//

#pragma once

#include "Utility/Debug.hpp"
#include "ECS/EntityManager.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "Core/Input.hpp"
#include "Core/AudioEngine.hpp"

#include "PlayerController.hpp"

namespace System
{
    // Arena boundaries (in world units)
    struct ArenaBounds
    {
        float minX = 1.0f;
        float minY = 1.0f;
        float maxX = 19.0f;
        float maxY = 10.25f;
    };

    inline void PlayerMovement(std::shared_ptr<EntityManager>& entityManager, float dt, const ArenaBounds& bounds = {})
    {
        auto& players = entityManager->getEntities("player");

        for (auto& e : players)
        {
            if (!e->hasComponent<Comp::PlayerController>() || !e->hasComponent<Comp::Transform>())
                continue;

            auto& controller = e->getComponent<Comp::PlayerController>();
            auto& transform = e->getComponent<Comp::Transform>();

            // Read input axes
            float moveX = 0.0f;
            float moveY = 0.0f;

            if (Input::isAction("moveLeft"))  moveX -= 1.0f;
            if (Input::isAction("moveRight")) moveX += 1.0f;
            if (Input::isAction("moveUp"))    moveY -= 1.0f;
            if (Input::isAction("moveDown"))  moveY += 1.0f;

            // Normalize diagonal movement
            if (moveX != 0.0f && moveY != 0.0f)
            {
                float invLen = 1.0f / std::sqrt(moveX * moveX + moveY * moveY);
                moveX *= invLen;
                moveY *= invLen;
            }

            // Apply velocity
            transform.velocity.x = moveX * controller.moveSpeed;
            transform.velocity.y = moveY * controller.moveSpeed;

            // Update position
            transform.position.x += transform.velocity.x * dt;
            transform.position.y += transform.velocity.y * dt;

            // Clamp to arena bounds
            float halfW = 0.5f;
            float halfH = 0.5f;
            transform.position.x = std::max(bounds.minX + halfW, std::min(bounds.maxX - halfW, transform.position.x));
            transform.position.y = std::max(bounds.minY + halfH, std::min(bounds.maxY - halfH, transform.position.y));

            // Track movement state for animation
            bool wasMoving = controller.isMoving;
            controller.isMoving = (moveX != 0.0f || moveY != 0.0f);

            // Update facing direction
            if (controller.isMoving)
            {
                if (std::abs(moveX) > std::abs(moveY))
                {
                    controller.facing = moveX > 0
                        ? Comp::PlayerController::Facing::RIGHT
                        : Comp::PlayerController::Facing::LEFT;
                }
                else
                {
                    controller.facing = moveY > 0
                        ? Comp::PlayerController::Facing::DOWN
                        : Comp::PlayerController::Facing::UP;
                }
            }

            // Switch between idle and walk sprite
            if (e->hasComponent<Comp::Sprite>())
            {
                auto& sprite = e->getComponent<Comp::Sprite>();

                if (controller.isMoving && !wasMoving)
                {
                    // Switched to walking
                    sprite.texture = AssetManager::getTexture("player_walk");
                    sprite.setAnimated(8.0f);
                }
                else if (!controller.isMoving && wasMoving)
                {
                    // Switched to idle
                    sprite.texture = AssetManager::getTexture("player");
                    sprite.type = Comp::Sprite::Type::Static;
                    sprite.currentFrame = 0;
                }

                // Flip sprite based on facing
                sprite.flipX(controller.facing == Comp::PlayerController::Facing::LEFT);
            }
        }
    }
}
