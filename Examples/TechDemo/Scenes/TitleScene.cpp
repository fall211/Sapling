//
//  TitleScene.cpp
//  Sapling TechDemo
//

#include "TitleScene.hpp"
#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Core/Input.hpp"
#include "ECS/Component.hpp"
#include "Renderer/Sprout.hpp"
#include "Utility/Color.hpp"
#include "Utility/Debug.hpp"

#include <string>

TitleScene::TitleScene(Engine& engine) : Scene(engine)
{
    init();
}

void TitleScene::init()
{
    spawnUI();
}

void TitleScene::update()
{
    // Blink the "Press Space" prompt
    m_blinkTimer += m_engine.deltaTime();
    if (m_blinkTimer >= 0.6f)
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

    // Animate floating entities
    System::FloatAnimator(m_entityManager, m_engine.deltaTime());

    // Check for start input
    if (Input::isActionUp("confirm"))
    {
        AudioEngine::playSound("scene_change");
        m_engine.changeScene("game");
    }

    if (Input::isActionUp("quit"))
    {
        sapp_request_quit();
    }

    // Render all entities using the default renderer
    sRender(m_entityManager->getEntities());
}

void TitleScene::onSceneEnabled()
{
    AudioEngine::playSound("bgm", true, 0.3f);
    m_blinkTimer = 0.0f;
    m_showPrompt = true;
}

void TitleScene::onSceneDisabled()
{
    AudioEngine::stopSound("bgm");
}

void TitleScene::spawnUI()
{
    // Title text
    auto title = m_entityManager->addEntity({"ui", "title"});
    title->addComponent<Comp::Transform>(glm::vec2(0, 80), Sprout::Pivot::TOP_CENTER, true);
    title->addComponent<Comp::Text>(
        "SAPLING TECH DEMO",
        "game_font", 24, Color::White,
        Sprout::TextJustify::CENTER
    );

    // Subtitle
    auto subtitle = m_entityManager->addEntity({"ui", "subtitle"});
    subtitle->addComponent<Comp::Transform>(glm::vec2(0, 40), Sprout::Pivot::TOP_CENTER, true);
    subtitle->addComponent<Comp::Text>(
        "A Top-Down Collector",
        "game_font", 12, Color::LightGray,
        Sprout::TextJustify::CENTER
    );

    // Animated player preview in the center of the screen
    auto playerPreview = m_entityManager->addEntity({"ui", "preview"});
    auto& previewTransform = playerPreview->addComponent<Comp::Transform>(glm::vec2(320, 220));
    previewTransform.pivot = Sprout::Pivot::CENTER;
    previewTransform.scale = glm::vec3(3.0f, 3.0f, 1.0f);
    auto& previewSprite = playerPreview->addComponent<Comp::Sprite>(
        AssetManager::getTexture("player_walk"), 8.0f
    );
    previewSprite.setLayer(Comp::Layer::Player);
    playerPreview->addComponent<Comp::FloatAnimation>(2.0f, 6.0f);

    // Floating gems around the player preview
    float gemPositions[][2] = {
        {240, 200}, {400, 200}, {260, 260}, {380, 260}, {320, 170}
    };

    for (int i = 0; i < 5; i++)
    {
        auto gem = m_entityManager->addEntity({"ui", "preview_gem"});
        auto& gemTransform = gem->addComponent<Comp::Transform>(
            glm::vec2(gemPositions[i][0], gemPositions[i][1])
        );
        gemTransform.pivot = Sprout::Pivot::CENTER;
        gemTransform.scale = glm::vec3(2.0f, 2.0f, 1.0f);
        auto& gemSprite = gem->addComponent<Comp::Sprite>(
            AssetManager::getTexture("gem_spin"), 6.0f
        );
        gemSprite.setLayer(Comp::Layer::Midground);
        gem->addComponent<Comp::FloatAnimation>();
    }

    // "Press Space to Start" prompt
    auto prompt = m_entityManager->addEntity({"ui", "prompt"});
    prompt->addComponent<Comp::Transform>(glm::vec2(0, -60), Sprout::Pivot::BOTTOM_CENTER, true);
    prompt->addComponent<Comp::Text>(
        "Press SPACE to Start",
        "game_font", 14, Color::White,
        Sprout::TextJustify::CENTER
    );

    // Controls hint
    auto controls = m_entityManager->addEntity({"ui", "controls"});
    controls->addComponent<Comp::Transform>(glm::vec2(0, -30), Sprout::Pivot::BOTTOM_CENTER, true);
    controls->addComponent<Comp::Text>(
        "WASD / Arrows to Move  |  ESC to Quit",
        "game_font", 8, Color::Gray,
        Sprout::TextJustify::CENTER
    );
}

bool TitleScene::onMessage(const SceneMessage& /*message*/)
{
    return false;
}