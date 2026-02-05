//
//  Scene.cpp
//  SaplingEngine, Canopy Scene Manager
//  

#include "Core/Scene.hpp"
#include "Core/AudioEngine.hpp"
#include "ECS/Component.hpp"
#include "Renderer/Sprout.hpp"
#include "Core/SceneMessage.hpp"
#include "Utility/Debug.hpp"


Scene::Scene(Engine& engine) : m_engine(engine)
{
    m_entityManager = std::make_shared<EntityManager>();
}


void Scene::preUpdate()
{
    m_entityManager->update();
    AudioEngine::update();
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
    Debug::log("enabling scene");
    onSceneEnabled();
}

void Scene::disable()
{
    Debug::log("disabling scene");
    onSceneDisabled();
}

void Scene::sRender(EntityList& entities)
{
    glm::f32 numEntities = entities.size();
    glm::f32 currentEnt = 0;

    float dt = m_engine.deltaTime();
    for (const auto& e : entities)
    {
        if (e->hasComponent<Comp::GridTransform>() && e->hasComponent<Comp::Transform>())
        {
            auto& gridTransform = e->getComponent<Comp::GridTransform>();
            auto& transform = e->getComponent<Comp::Transform>();
            transform.position = gridTransform.getWorldPosition();
        }
        
        if (e->hasComponentEnabled<Comp::Sprite>())
        {
            
            auto& cSprite = e->getComponent<Comp::Sprite>();

            if (cSprite.type == Comp::Sprite::Type::Animated)
            {
                // update the animation frame based on delta time
                cSprite.animationTime += dt;
                if (cSprite.animationTime >= (1.0f / cSprite.animationSpeed))
                {
                    cSprite.currentFrame = (cSprite.currentFrame + 1) % cSprite.numFrames;
                    cSprite.animationTime = 0.0f;
                }
            }
            glm::f32 depth = 1 - (static_cast<glm::f32>(cSprite.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);
            glm::vec2 pos = glm::vec2(0.0f);
            glm::vec3 scale = glm::vec3(1.0f);
            glm::f32 rotation = 0.0f;
            Sprout::Pivot pivot = Sprout::Pivot::CENTER;
            bool worldSpace = true;
            
            if (e->hasComponent<Comp::Transform>())
            {
                auto& cTransform = e->getComponent<Comp::Transform>();
                pos = cTransform.position + cSprite.transformOffset;
                scale = cTransform.scale * cSprite.scaleOffset;
                rotation = cTransform.rotation;
                pivot = cTransform.pivot;
            }
            else if (e->hasComponent<Comp::GUITransform>())
            {
                auto& cUITransform = e->getComponent<Comp::GUITransform>();
                worldSpace = false;
                pos = cUITransform.screenPosition;
                pivot = cUITransform.pivot;
                scale = cUITransform.scale;
            }
            
            if (cSprite.flip_X)
            {
                scale.x *= -1;
            }
            
            m_engine.getWindow().draw_sprite(cSprite.texture, pos, depth, rotation, (int)cSprite.currentFrame, cSprite.color_override, scale, pivot, worldSpace);
            
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
        if (e->hasComponentEnabled<Comp::Text>())
        {
            auto& cText = e->getComponent<Comp::Text>();
            
            glm::vec2 pos = glm::vec2(0.0f);
            float scale = 0.025f * cText.size;
            Sprout::Pivot pivot = Sprout::Pivot::TOP_LEFT;
            bool worldSpace = true;
            float depth = 1 - (static_cast<glm::f32>(cText.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);
            
            if (e->hasComponent<Comp::Transform>())
            {
                auto& cTransform = e->getComponent<Comp::Transform>();
                pos = cTransform.position + cText.transformOffset;
                pivot = cTransform.pivot;
            }
            else if (e->hasComponent<Comp::GUITransform>())
            {
                auto& cUITransform = e->getComponent<Comp::GUITransform>();
                worldSpace = false;
                pos = cUITransform.screenPosition + cText.transformOffset;
                pivot = cUITransform.pivot;
            }
            
            
            m_engine.getWindow().draw_text(cText.text, AssetManager::getFont(cText.font), pos, depth, cText.color, scale, pivot, worldSpace, cText.justify);
        }
        
        if (e->hasComponentEnabled<Comp::Image>())
        {
            auto& image = e->getComponent<Comp::Image>();
            glm::vec2 pos = glm::vec2(0,0);
            glm::f32 depth = 1 - (static_cast<glm::f32>(image.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);
            glm::f32 rotation = 0.0f;
            glm::vec3 scale = glm::vec3(1);
            Sprout::Pivot pivot = Sprout::Pivot::TOP_LEFT;
            
            if (e->hasComponent<Comp::Transform>())
            {
                auto& transform = e->getComponent<Comp::Transform>();
                pos = transform.position + image.transformOffset;
                scale = transform.scale * image.scaleOffset;
                rotation = transform.rotation;
                pivot = transform.pivot;
            }
            m_engine.getWindow().draw_standalone_texture(image.texture, pos, depth, rotation, scale, pivot);
        }
        currentEnt++;
    }
}