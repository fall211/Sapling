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
#include "Core/AssetManager.hpp"

#include "PlayerController.hpp"

#include <algorithm>
#include <cmath>

namespace System
{
    // Arena boundaries (in world units). +Y is down.
    struct ArenaBounds
    {
        float minX = 1.0f;
        float minY = 1.0f;
        float maxX = 19.0f;
        float maxY = 10.25f;
        float floorY = 10.0f;
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

            float halfW = 0.375f;
            float halfH = 0.375f;
            if (e->hasComponent<Comp::BBox>())
            {
                const auto& box = e->getComponent<Comp::BBox>();
                halfW = box.w * 0.5f;
                halfH = box.h * 0.5f;
            }

            float moveX = 0.0f;
            if (Input::isAction("moveLeft"))  moveX -= 1.0f;
            if (Input::isAction("moveRight")) moveX += 1.0f;

            transform.velocity.x = moveX * controller.moveSpeed;

            if (controller.grounded)
            {
                controller.coyoteLeft = controller.coyoteTime;
            }
            else
            {
                controller.coyoteLeft = std::max(0.0f, controller.coyoteLeft - dt);
            }

            const bool jumpDown = Input::isAction("jump");
            const bool canJump = controller.grounded || controller.coyoteLeft > 0.0f;
            if (jumpDown && !controller.jumpHeld && canJump)
            {
                transform.velocity.y = -controller.jumpSpeed;
                controller.grounded = false;
                controller.coyoteLeft = 0.0f;
            }
            controller.jumpHeld = jumpDown;

            if (!jumpDown && transform.velocity.y < 0.0f)
            {
                transform.velocity.y *= controller.jumpCut;
            }

            const float step = std::min(dt, 1.0f / 30.0f);
            transform.velocity.y += controller.gravity * step;

            transform.position.x += transform.velocity.x * dt;
            transform.position.y += transform.velocity.y * step;

            transform.position.x = std::max(bounds.minX + halfW, std::min(bounds.maxX - halfW, transform.position.x));
            transform.position.y = std::max(bounds.minY + halfH, transform.position.y);

            const float feet = transform.position.y + halfH;
            if (transform.velocity.y >= 0.0f && feet >= bounds.floorY)
            {
                transform.position.y = bounds.floorY - halfH;
                transform.velocity.y = 0.0f;
                controller.grounded = true;
            }
            else
            {
                controller.grounded = false;
            }

            bool wasMoving = controller.isMoving;
            controller.isMoving = (moveX != 0.0f) && controller.grounded;

            if (moveX != 0.0f)
            {
                controller.facing = moveX > 0
                    ? Comp::PlayerController::Facing::RIGHT
                    : Comp::PlayerController::Facing::LEFT;
            }

            if (e->hasComponent<Comp::Sprite>())
            {
                auto& sprite = e->getComponent<Comp::Sprite>();

                if (controller.isMoving && !wasMoving)
                {
                    sprite.texture = AssetManager::getTexture("player_walk");
                    sprite.setAnimated(8.0f);
                }
                else if (!controller.isMoving && wasMoving)
                {
                    sprite.texture = AssetManager::getTexture("player");
                    sprite.type = Comp::Sprite::Type::Static;
                    sprite.currentFrame = 0;
                }

                sprite.flipX(controller.facing == Comp::PlayerController::Facing::LEFT);
            }
        }
    }
}
