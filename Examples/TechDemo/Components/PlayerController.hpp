//
//  PlayerController.hpp
//  Sapling TechDemo
//

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "Utility/Debug.hpp"

#include <utility>

namespace Comp
{
    struct PlayerController : public Component
    {
        float moveSpeed = 5.0f;
        float gravity = 55.0f;
        float jumpSpeed = 14.0f;
        float coyoteTime = 0.1f;
        float jumpCut = 0.45f;
        float coyoteLeft = 0.0f;
        int score = 0;
        int health = 3;
        bool isMoving = false;
        bool grounded = false;
        bool jumpHeld = false;
        float dashSpeed = 13.0f;
        float dashTime = 0.10f;
        float dashCooldown = 0.22f;
        float dashCoast = 20.0f;
        float dashLeft = 0.0f;
        float dashCooldownLeft = 0.0f;
        float dashDir = 1.0f;
        float climbSpeed = 6.0f;
        float wallJumpSpeed = 11.0f;
        float wallDir = 0.0f;
        bool onWall = false;

        enum class Facing : uint8_t
        {
            UP,
            DOWN,
            LEFT,
            RIGHT
        } facing = Facing::RIGHT;

        explicit PlayerController(Inst inst)
            : Component(std::move(inst))
        {}

        explicit PlayerController(Inst inst, float speed)
            : Component(std::move(inst)), moveSpeed(speed)
        {}

        void OnAddToEntity() override
        {
            inst->requestAddTag("player");
        }

        void OnRemoveFromEntity() override
        {
            inst->requestRemoveTag("player");
        }
    };
}
