//
//  ScoreScene.cpp
//  Sapling TechDemo
//

#include "ScoreScene.hpp"
#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Core/Input.hpp"
#include "ECS/Component.hpp"
#include "Renderer/Sprout.hpp"
#include "Utility/Color.hpp"

#include <string>

ScoreScene::ScoreScene(Engine& engine) : Scene(engine)
{
    init();
}

void ScoreScene::init()
{
    spawnUI();
}

void ScoreScene::update()
{
    float dt = m_engine.deltaTime();

    m_blinkTimer += dt;
    if (m_blinkTimer >= 0.5f)
    {
        m_blinkTimer = 0.0f;
        m_showPrompt = !m_showPrompt;

        auto& prompts = m_entityManager->getEntities("prompt");
        for (auto& p : prompts)
        {
            if (p->hasComponent<Comp::Text>())
            {
                p->getComponent<Comp::Text>().color = m_showPrompt
                    ? Color::White
                    : Color::Transparent;
            }
        }
    }

    System::FloatMotion(m_entityManager, dt);

    if (Input::isActionUp("confirm"))
    {
        AudioEngine::playSound("scene_change");
        m_engine.changeScene("game");
    }

    if (Input::isActionUp("quit"))
    {
        AudioEngine::playSound("scene_change");
        m_engine.changeScene("title");
    }

    sRender(m_entityManager->getEntities());
}

void ScoreScene::onSceneEnabled()
{
    m_blinkTimer = 0.0f;
    m_showPrompt = true;
    AudioEngine::playSound("score_jingle", false, 0.5f);
}

void ScoreScene::onSceneDisabled()
{
    AudioEngine::stopSound("score_jingle");
}

void ScoreScene::spawnUI()
{
    auto header = m_entityManager->addEntity({"ui", "header"});
    header->addComponent<Comp::Transform>(glm::vec2(0, 160), Sprout::Pivot::TOP_CENTER, true);
    header->addComponent<Comp::Text>(
        "YOU WON",
        "game_font", 32, Color::Gold,
        Sprout::TextJustify::CENTER
    );

    auto sub = m_entityManager->addEntity({"ui", "subtitle"});
    sub->addComponent<Comp::Transform>(glm::vec2(0, 230), Sprout::Pivot::TOP_CENTER, true);
    sub->addComponent<Comp::Text>(
        "Three gems. The heart. Done.",
        "game_font", 14, Color::White,
        Sprout::TextJustify::CENTER
    );

    auto prompt = m_entityManager->addEntity({"ui", "prompt"});
    prompt->addComponent<Comp::Transform>(glm::vec2(0, 120), Sprout::Pivot::BOTTOM_CENTER, true);
    prompt->addComponent<Comp::Text>(
        "Press SPACE to Play Again",
        "game_font", 14, Color::White,
        Sprout::TextJustify::CENTER
    );

    auto hint = m_entityManager->addEntity({"ui", "hint"});
    hint->addComponent<Comp::Transform>(glm::vec2(0, 70), Sprout::Pivot::BOTTOM_CENTER, true);
    hint->addComponent<Comp::Text>(
        "Press ESC for Title Screen",
        "game_font", 8, Color::Gray,
        Sprout::TextJustify::CENTER
    );
}

void ScoreScene::updateScoreDisplay()
{
}

bool ScoreScene::onMessage(const SceneMessage& /*message*/)
{
    return false;
}
