//
//  Collectible.hpp
//  Sapling TechDemo
//

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "Utility/Debug.hpp"

#include <utility>

namespace Comp
{
    struct Collectible : public Component
    {
        int pointValue = 10;
        bool collected = false;
        float collectRadius = 0.875f;

        explicit Collectible(Inst inst)
            : Component(std::move(inst))
        {}

        explicit Collectible(Inst inst, int points)
            : Component(std::move(inst)), pointValue(points)
        {}

        explicit Collectible(Inst inst, int points, float radius)
            : Component(std::move(inst)), pointValue(points), collectRadius(radius)
        {}

        void OnAddToEntity() override
        {
            inst->requestAddTag("collectible");
        }

        void OnRemoveFromEntity() override
        {
            inst->requestRemoveTag("collectible");
        }
    };
}
