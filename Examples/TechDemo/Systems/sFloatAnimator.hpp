//
//  sFloatAnimator.hpp
//  Sapling TechDemo
//

#pragma once

#include "Utility/Debug.hpp"
#include "ECS/EntityManager.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"

#include "FloatAnimation.hpp"

namespace System
{
    inline void FloatAnimator(std::shared_ptr<EntityManager>& entityManager, float dt)
    {
        auto& entities = entityManager->getEntities("floatAnim");

        for (auto& e : entities)
        {
            if (!e->hasComponent<Comp::FloatAnimation>())
                continue;

            auto& floatAnim = e->getComponent<Comp::FloatAnimation>();
            float offsetY = floatAnim.evaluate(dt);

            if (e->hasComponent<Comp::Sprite>())
            {
                auto& sprite = e->getComponent<Comp::Sprite>();
                sprite.transformOffset.y = offsetY;
            }
            else if (e->hasComponent<Comp::Image>())
            {
                auto& image = e->getComponent<Comp::Image>();
                image.transformOffset.y = offsetY;
            }
        }
    }
}