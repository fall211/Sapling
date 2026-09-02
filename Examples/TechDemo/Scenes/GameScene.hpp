//
//  GameScene.hpp
//  Sapling TechDemo
//

#pragma once

#include "Core/Scene.hpp"
#include "Utility/Color.hpp"

#include "sPlayerMovement.hpp"

#include "PlayerController.hpp"

class GameScene : public Scene
{
private:
    static constexpr int ARENA_COLS = 20;
    static constexpr int ARENA_ROWS = 12;
    static constexpr float TILE_SIZE = 1.0f;
    static constexpr float ARENA_W = 20.0f;
    static constexpr float ARENA_H = 11.25f;

    System::ArenaBounds m_bounds;

public:
    GameScene(Engine& engine);
    ~GameScene() = default;

    void init() override;
    void update() override;
    void onSceneEnabled() override;
    void onSceneDisabled() override;

    void spawnArena();
    void spawnSolidTile(int tx, int ty, bool wall);
    void spawnPlatform(int tx0, int tx1, int ty);
    void spawnPlayer();
    void spawnHUD();
    void resetGame();

    bool onMessage(const SceneMessage& message) override;
};
