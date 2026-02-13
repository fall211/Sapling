//
//  ScoreScene.hpp
//  Sapling TechDemo
//

#pragma once

#include "Core/Scene.hpp"
#include "Utility/Color.hpp"

#include "sFloatAnimator.hpp"
#include "FloatAnimation.hpp"

class ScoreScene : public Scene
{
private:
    int m_finalScore = 0;
    float m_blinkTimer = 0.0f;
    bool m_showPrompt = true;
    bool m_jinglePlayed = false;

public:
    ScoreScene(Engine& engine);
    ~ScoreScene() = default;

    void init() override;
    void update() override;
    void onSceneEnabled() override;
    void onSceneDisabled() override;

    void spawnUI();
    void updateScoreDisplay();

    bool onMessage(const SceneMessage& message) override;
};