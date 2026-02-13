//
//  TitleScene.hpp
//  Sapling TechDemo
//

#pragma once

#include "Core/Scene.hpp"
#include "Utility/Color.hpp"

#include "sFloatAnimator.hpp"
#include "FloatAnimation.hpp"

class TitleScene : public Scene
{
private:
    float m_blinkTimer = 0.0f;
    bool m_showPrompt = true;

public:
    TitleScene(Engine& engine);
    ~TitleScene() = default;

    void init() override;
    void update() override;
    void onSceneEnabled() override;
    void onSceneDisabled() override;

    void spawnUI();

    bool onMessage(const SceneMessage& message) override;
};