//
//  Demo3DScene.hpp
//  Sapling Demo3D
//

#pragma once

#include "Core/Scene.hpp"

#include <memory>

class Demo3DScene : public Scene
{
private:
    float m_orbitAngle = 0.0f;
    float m_orbitRadius = 6.0f;
    float m_orbitHeight = 4.0f;
    float m_orbitSpeed = 0.5f;

    float m_cubeRotation = 0.0f;
    float m_pointLightAngle = 0.0f;

    std::shared_ptr<Entity> m_characterEntity;
    std::shared_ptr<Entity> m_pointLightEntity;
    std::shared_ptr<Entity> m_antLabelEntity;

public:
    Demo3DScene(Engine& engine);
    ~Demo3DScene() = default;

    void init() override;
    void update() override;
    void onSceneEnabled() override;
};
