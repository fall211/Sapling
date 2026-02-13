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
        float moveSpeed = 120.0f;
        int score = 0;
        int health = 3;
        bool isMoving = false;

        enum class Facing : uint8_t
        {
            UP,
            DOWN,
            LEFT,
            RIGHT
        } facing = Facing::DOWN;

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