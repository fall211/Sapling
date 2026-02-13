//
//  sCollectibles.hpp
//  Sapling TechDemo
//

#pragma once

#include "Utility/Debug.hpp"
#include "ECS/EntityManager.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "Core/AudioEngine.hpp"

#include "PlayerController.hpp"
#include "Collectible.hpp"

#include "glm/geometric.hpp"

namespace System
{
    struct CollectibleResult
    {
        int totalCollected = 0;
        int scoreGained = 0;
    };

    inline CollectibleResult CheckCollectibles(std::shared_ptr<EntityManager>& entityManager)
    {
        CollectibleResult result;

        auto& players = entityManager->getEntities("player");
        auto& collectibles = entityManager->getEntities("collectible");

        if (players.empty() || collectibles.empty())
            return result;

        for (auto& player : players)
        {
            if (!player->hasComponent<Comp::Transform>() || !player->hasComponent<Comp::PlayerController>())
                continue;

            auto& playerTransform = player->getComponent<Comp::Transform>();
            auto& controller = player->getComponent<Comp::PlayerController>();
            glm::vec2 playerPos = glm::vec2(playerTransform.position);

            for (auto& gem : collectibles)
            {
                if (!gem->hasComponent<Comp::Transform>() || !gem->hasComponent<Comp::Collectible>())
                    continue;

                auto& collectible = gem->getComponent<Comp::Collectible>();

                if (collectible.collected)
                    continue;

                auto& gemTransform = gem->getComponent<Comp::Transform>();
                glm::vec2 gemPos = glm::vec2(gemTransform.position);

                // Apply sprite offset if the gem has a float animation offset
                if (gem->hasComponent<Comp::Sprite>())
                {
                    gemPos += gem->getComponent<Comp::Sprite>().transformOffset;
                }

                // Check distance-based overlap
                float dist = glm::distance(playerPos, gemPos);
                if (dist < collectible.collectRadius)
                {
                    // Collect it
                    collectible.collected = true;
                    controller.score += collectible.pointValue;

                    result.totalCollected++;
                    result.scoreGained += collectible.pointValue;

                    // Play collection sound
                    AudioEngine::playSound("collect");

                    // Mark for destruction
                    gem->destroy();

                    Debug::log("Collected gem! Score: " + std::to_string(controller.score));
                }
            }
        }

        return result;
    }

    inline int CountRemainingCollectibles(std::shared_ptr<EntityManager>& entityManager)
    {
        auto& collectibles = entityManager->getEntities("collectible");
        int count = 0;
        for (auto& gem : collectibles)
        {
            if (gem->hasComponent<Comp::Collectible>() && !gem->getComponent<Comp::Collectible>().collected)
            {
                count++;
            }
        }
        return count;
    }
}