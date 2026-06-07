//
//  sFloatMotion.hpp
//  Sapling TechDemo
//

#pragma once

#include "Utility/Debug.hpp"
#include "ECS/EntityManager.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"

#include "FloatMotion.hpp"

namespace System
{
    inline void FloatMotion(std::shared_ptr<EntityManager>& entityManager, float dt)
    {
        auto& entities = entityManager->getEntities("floatMotion");

        for (auto& e : entities)
        {
            if (!e->hasComponent<Comp::FloatMotion>())
                continue;

            auto& floatMotion = e->getComponent<Comp::FloatMotion>();
            float offsetY = floatMotion.evaluate(dt);

            if (e->hasComponent<Comp::Sprite>())
            {
                auto& sprite = e->getComponent<Comp::Sprite>();
                sprite.transformOffset.y = offsetY;
            }
        }
    }
}