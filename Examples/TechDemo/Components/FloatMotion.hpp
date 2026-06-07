//
//  FloatMotion.hpp
//  Sapling TechDemo
//

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "Utility/Debug.hpp"

#include "glm/ext/scalar_constants.hpp"
#include "glm/trigonometric.hpp"

#include <cstdlib>
#include <utility>

namespace Comp
{
    struct FloatMotion : public Component
    {
        float time = 0.0f;
        float speed = 2.0f;
        float amplitude = 4.0f;
        bool randomize = true;

        explicit FloatMotion(Inst inst)
            : Component(std::move(inst))
        {
            speed = (static_cast<float>(rand()) / RAND_MAX) * 2.0f + 1.0f;
            time = (static_cast<float>(rand()) / RAND_MAX) * glm::pi<float>() * 2.0f;
        }

        explicit FloatMotion(Inst inst, float speedIn, float amplitudeIn)
            : Component(std::move(inst)), speed(speedIn), amplitude(amplitudeIn), randomize(false)
        {}

        float evaluate(float dt)
        {
            time += dt;
            return glm::sin(time * speed) * amplitude;
        }

        float easeInOutSine(float x)
        {
            return -(glm::cos(glm::pi<float>() * x) - 1.0f) / 2.0f;
        }

        void OnAddToEntity() override
        {
            inst->requestAddTag("floatMotion");
        }

        void OnRemoveFromEntity() override
        {
            inst->requestRemoveTag("floatMotion");
        }
    };
}