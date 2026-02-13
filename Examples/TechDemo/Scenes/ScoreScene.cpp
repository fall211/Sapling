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
#include "Utility/Debug.hpp"

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

    // Blink the prompt text
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

    // Animate floating elements
    System::FloatAnimator(m_entityManager, dt);

    // Handle input
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

    // Render
    sRender(m_entityManager->getEntities());
}

void ScoreScene::onSceneEnabled()
{
    m_blinkTimer = 0.0f;
    m_showPrompt = true;
    m_jinglePlayed = false;

    // Play the score jingle
    if (!m_jinglePlayed)
    {
        AudioEngine::playSound("score_jingle", false, 0.5f);
        m_jinglePlayed = true;
    }

    // Update the score display with the received score
    updateScoreDisplay();
}

void ScoreScene::onSceneDisabled()
{
    AudioEngine::stopSound("score_jingle");
}

void ScoreScene::spawnUI()
{
    // "RESULTS" header
    auto header = m_entityManager->addEntity({"ui", "header"});
    header->addComponent<Comp::Transform>(glm::vec2(0, 80), Sprout::Pivot::TOP_CENTER, true);
    header->addComponent<Comp::Text>(
        "RESULTS",
        "game_font", 28, Color::Gold,
        Sprout::TextJustify::CENTER
    );

    // Score value display
    auto scoreLabel = m_entityManager->addEntity({"ui", "score_label"});
    scoreLabel->addComponent<Comp::Transform>(glm::vec2(0, 40), Sprout::Pivot::TOP_CENTER, true);
    scoreLabel->addComponent<Comp::Text>(
        "Your Score",
        "game_font", 12, Color::LightGray,
        Sprout::TextJustify::CENTER
    );

    auto scoreValue = m_entityManager->addEntity({"ui", "score_value"});
    scoreValue->addComponent<Comp::Transform>(glm::vec2(0, 10), Sprout::Pivot::TOP_CENTER, true);
    scoreValue->addComponent<Comp::Text>(
        "0",
        "game_font", 32, Color::White,
        Sprout::TextJustify::CENTER
    );

    // Rating text based on score
    auto rating = m_entityManager->addEntity({"ui", "rating"});
    rating->addComponent<Comp::Transform>(glm::vec2(0, -40), Sprout::Pivot::CENTER, true);
    rating->addComponent<Comp::Text>(
        "",
        "game_font", 14, Color::Yellow,
        Sprout::TextJustify::CENTER
    );

    // Decorative floating gems around the score
    float gemPositions[][2] = {
        {160, 180}, {480, 180}, {120, 300}, {520, 300},
        {200, 350}, {440, 350}
    };

    for (int i = 0; i < 6; i++)
    {
        auto gem = m_entityManager->addEntity({"ui", "deco_gem"});
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

    // "Press Space to Play Again" prompt
    auto prompt = m_entityManager->addEntity({"ui", "prompt"});
    prompt->addComponent<Comp::Transform>(glm::vec2(0, -70), Sprout::Pivot::BOTTOM_CENTER, true);
    prompt->addComponent<Comp::Text>(
        "Press SPACE to Play Again",
        "game_font", 14, Color::White,
        Sprout::TextJustify::CENTER
    );

    // "Press ESC for Title Screen" hint
    auto hint = m_entityManager->addEntity({"ui", "hint"});
    hint->addComponent<Comp::Transform>(glm::vec2(0, -40), Sprout::Pivot::BOTTOM_CENTER, true);
    hint->addComponent<Comp::Text>(
        "Press ESC for Title Screen",
        "game_font", 8, Color::Gray,
        Sprout::TextJustify::CENTER
    );
}

void ScoreScene::updateScoreDisplay()
{
    // Update the score value text
    auto& scoreValues = m_entityManager->getEntities("score_value");
    if (!scoreValues.empty() && scoreValues.front()->hasComponent<Comp::Text>())
    {
        scoreValues.front()->getComponent<Comp::Text>().text = std::to_string(m_finalScore);
    }

    // Update the rating text based on score
    auto& ratings = m_entityManager->getEntities("rating");
    if (!ratings.empty() && ratings.front()->hasComponent<Comp::Text>())
    {
        auto& ratingText = ratings.front()->getComponent<Comp::Text>();

        if (m_finalScore >= 500)
        {
            ratingText.text = "AMAZING!";
            ratingText.color = Color::Gold;
        }
        else if (m_finalScore >= 300)
        {
            ratingText.text = "Great Job!";
            ratingText.color = Color::Lime;
        }
        else if (m_finalScore >= 150)
        {
            ratingText.text = "Not Bad!";
            ratingText.color = Color::SkyBlue;
        }
        else if (m_finalScore > 0)
        {
            ratingText.text = "Keep Trying!";
            ratingText.color = Color::Orange;
        }
        else
        {
            ratingText.text = "Better Luck Next Time";
            ratingText.color = Color::LightGray;
        }
    }
}

bool ScoreScene::onMessage(const SceneMessage& message)
{
    if (message.hasType("final_score"))
    {
        m_finalScore = message.getData<int>();
        Debug::log("Received final score: " + std::to_string(m_finalScore));
        updateScoreDisplay();
        return true;
    }

    return false;
}