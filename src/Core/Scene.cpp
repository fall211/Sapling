//
//  Scene.cpp
//  SaplingEngine, Canopy Scene Manager
//

#include "Core/Scene.hpp"
#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "ECS/Component.hpp"
#include "Renderer/Sprout.hpp"
#include "Core/SceneMessage.hpp"
#include "Core/Logger.hpp"

namespace
{
    float effectivePixelsPerUnit(const Comp::Sprite& sprite)
    {
        float pixelsPerUnit = AssetManager::getPixelsPerUnit();

        if (sprite.texture && sprite.texture->hasPixelsPerUnitOverride())
        {
            pixelsPerUnit = sprite.texture->getPixelsPerUnit();
        }

        if (sprite.pixelsPerUnit > 0.0f)
        {
            pixelsPerUnit = sprite.pixelsPerUnit;
        }

        return pixelsPerUnit > 0.0f ? pixelsPerUnit : 16.0f;
    }
}

Scene::Scene(Engine& engine) : m_engine(engine)
{
    m_entityManager = std::make_shared<EntityManager>();
}


void Scene::preUpdate()
{
    m_entityManager->update();
    AudioEngine::update();
    sUpdateAnimations(m_entityManager->getEntities());
}

void Scene::sUpdateAnimations(EntityList& entities)
{
    float dt = m_engine.deltaTime();

    for (const auto& e : entities)
    {
        if (e->hasComponentEnabled<Comp::Sprite>())
        {
            auto& cSprite = e->getComponent<Comp::Sprite>();

            if (cSprite.type == Comp::Sprite::Type::Animated)
            {
                cSprite.animationTime += dt;
                if (cSprite.animationTime >= (1.0f / cSprite.animationSpeed))
                {
                    cSprite.currentFrame = (cSprite.currentFrame + 1) % cSprite.numFrames;
                    cSprite.animationTime = 0.0f;
                }
            }

            if (cSprite.colorOverrideTime > 0)
            {
                cSprite.colorOverrideTime -= dt;
                if (cSprite.colorOverrideTime <= 0)
                {
                    cSprite.color_override = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
                    cSprite.colorOverrideTime = 0;
                }
            }
        }
    }
}

void Scene::postUpdate()
{
    Input::clean();
}

void Scene::onSceneEnabled(){}
void Scene::onSceneDisabled(){}

void Scene::enable()
{
    // set the window's event callback to our input system
    m_engine.getWindow().SetEventCallback([](const sapp_event* e) { Input::update(e); });
    Logger::info("Enabling scene");
    onSceneEnabled();
}

void Scene::disable()
{
    Logger::info("Disabling scene");
    onSceneDisabled();
}

void Scene::sRender(EntityList& entities)
{
    glm::f32 numEntities = entities.size();
    glm::f32 currentEnt = 0;

    for (const auto& e : entities)
    {
        if (e->hasComponent<Comp::GridTransform>() && e->hasComponent<Comp::Transform>())
        {
            auto& gridTransform = e->getComponent<Comp::GridTransform>();
            auto& transform = e->getComponent<Comp::Transform>();
            auto worldPos = gridTransform.getWorldPosition();
            transform.position = glm::vec3(worldPos, transform.position.z);
        }

        if (e->hasComponentEnabled<Comp::Sprite>() && e->getComponent<Comp::Sprite>().texture)
        {
            auto& cSprite = e->getComponent<Comp::Sprite>();

            glm::f32 depth = 1 - (static_cast<glm::f32>(cSprite.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);
            glm::vec2 pos = glm::vec2(0.0f);
            glm::vec3 scale = glm::vec3(1.0f);
            glm::f32 rotation = 0.0f;
            Sprout::Pivot pivot = Sprout::Pivot::CENTER;
            bool worldSpace = true;

            if (e->hasComponent<Comp::Transform>())
            {
                auto& cTransform = e->getComponent<Comp::Transform>();
                pos = glm::vec2(cTransform.position) + cSprite.transformOffset;
                scale = cTransform.scale * cSprite.scaleOffset;
                rotation = cTransform.rotation.z;
                pivot = cTransform.pivot;
                worldSpace = !cTransform.screenSpace;
            }

            if (worldSpace)
            {
                scale *= 1.0f / effectivePixelsPerUnit(cSprite);
            }

            if (cSprite.flip_X)
            {
                scale.x *= -1;
            }

            // independent mode textures (not atlas-packed) must use draw_image
            if (cSprite.texture->getMode() == Sprout::TextureMode::Independent) {
                m_engine.getWindow().draw_image(cSprite.texture, pos, depth, rotation, scale, pivot, cSprite.color_override, worldSpace);
            } else {
                m_engine.getWindow().draw_sprite(cSprite.texture, pos, depth, rotation, (int)cSprite.currentFrame, cSprite.color_override, scale, pivot, worldSpace);
            }
        }
        if (e->hasComponentEnabled<Comp::Text>())
        {
            auto& cText = e->getComponent<Comp::Text>();

            if (!cText.font.empty() && AssetManager::hasFont(cText.font))
            {
                glm::vec2 pos = glm::vec2(0.0f);
                float scale = 0.025f * cText.size;
                Sprout::Pivot pivot = Sprout::Pivot::TOP_LEFT;
                bool worldSpace = true;
                float depth = 1 - (static_cast<glm::f32>(cText.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);

                if (e->hasComponent<Comp::Transform>())
                {
                    auto& cTransform = e->getComponent<Comp::Transform>();
                    pos = glm::vec2(cTransform.position) + cText.transformOffset;
                    pivot = cTransform.pivot;
                    worldSpace = !cTransform.screenSpace;
                    scale *= cTransform.scale.x;
                }

                m_engine.getWindow().draw_text(cText.text, AssetManager::getFont(cText.font), pos, depth, cText.color, scale, pivot, worldSpace, cText.justify);
            }
        }

        currentEnt++;
    }
}

void Scene::render()
{
    auto& entities = m_entityManager->getEntities();
    sRender(entities);
}
