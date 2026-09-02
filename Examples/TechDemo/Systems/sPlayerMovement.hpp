//
//  sPlayerMovement.hpp
//  Sapling TechDemo
//

#pragma once

#include "ECS/EntityManager.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "Core/Input.hpp"
#include "Core/AssetManager.hpp"

#include "PlayerController.hpp"

#include <algorithm>
#include <vector>
#include <cmath>

namespace System
{
    struct ArenaBounds
    {
        float minX = 1.0f;
        float minY = 1.0f;
        float maxX = 19.0f;
        float maxY = 10.25f;
    };

    struct SolidBox
    {
        float x = 0.0f;
        float y = 0.0f;
        float w = 1.0f;
        float h = 1.0f;
    };

    inline bool overlapX(float ax, float aw, float bx, float bw)
    {
        return ax < bx + bw && ax + aw > bx;
    }

    inline bool overlapY(float ay, float ah, float by, float bh)
    {
        return ay < by + bh && ay + ah > by;
    }

    inline void PlayerMovement(std::shared_ptr<EntityManager>& entityManager, float dt, const ArenaBounds& bounds = {})
    {
        auto collect = [&](const char* tag) {
            std::vector<SolidBox> out;
            for (auto& tile : entityManager->getEntities(tag))
            {
                if (!tile->hasComponent<Comp::Transform>())
                    continue;
                const auto& tf = tile->getComponent<Comp::Transform>();
                float w = 1.0f;
                float h = 1.0f;
                if (tile->hasComponent<Comp::BBox>())
                {
                    const auto& box = tile->getComponent<Comp::BBox>();
                    w = box.w;
                    h = box.h;
                }
                out.push_back({tf.position.x, tf.position.y, w, h});
            }
            return out;
        };
        const auto walls = collect("wall");
        const auto solids = collect("solid");

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
            const float px = transform.position.x - halfW;
            const float py = transform.position.y - halfH;
            for (const auto& s : walls)
            {
                if (!overlapX(px, halfW * 2.0f, s.x, s.w))
                    continue;
                if (!overlapY(py + 0.05f, halfH * 2.0f - 0.05f, s.y, s.h))
                    continue;
                const float tileMid = s.x + s.w * 0.5f;
                if (transform.position.x < tileMid)
                {
                    transform.position.x = s.x - halfW;
                }
                else
                {
                    transform.position.x = s.x + s.w + halfW;
                }
            }

            transform.position.y += transform.velocity.y * step;
            controller.grounded = false;
            {
                const float px2 = transform.position.x - halfW;
                const float feet = transform.position.y + halfH;
                if (transform.velocity.y >= 0.0f)
                {
                    for (const auto& s : solids)
                    {
                        if (!overlapX(px2 + 0.04f, halfW * 2.0f - 0.08f, s.x, s.w))
                            continue;
                        if (feet >= s.y && feet <= s.y + 0.5f)
                        {
                            transform.position.y = s.y - halfH;
                            transform.velocity.y = 0.0f;
                            controller.grounded = true;
                            break;
                        }
                    }
                }
                else
                {
                    const float py2 = transform.position.y - halfH;
                    for (const auto& s : walls)
                    {
                        if (!overlapX(px2 + 0.04f, halfW * 2.0f - 0.08f, s.x, s.w))
                            continue;
                        if (!overlapY(py2, halfH * 2.0f, s.y, s.h))
                            continue;
                        transform.position.y = s.y + s.h + halfH;
                        transform.velocity.y = 0.0f;
                    }
                }
            }

            transform.position.x = std::max(bounds.minX + halfW, std::min(bounds.maxX - halfW, transform.position.x));
            transform.position.y = std::max(bounds.minY + halfH, std::min(bounds.maxY + 2.0f, transform.position.y));

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
