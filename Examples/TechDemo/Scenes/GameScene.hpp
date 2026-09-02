//
//  GameScene.hpp
//  Sapling TechDemo
//

#pragma once

#include "Core/Scene.hpp"
#include "Utility/Color.hpp"

#include "sPlayerMovement.hpp"
#include "sCollectibles.hpp"
#include "sFloatMotion.hpp"
#include "sEnemyAI.hpp"

#include "PlayerController.hpp"
#include "Collectible.hpp"
#include "FloatMotion.hpp"
#include "EnemyAI.hpp"

class GameScene : public Scene
{
private:
    static constexpr int ARENA_COLS = 20;
    static constexpr int ARENA_ROWS = 12;
    static constexpr float TILE_SIZE = 1.0f;
    static constexpr float ARENA_W = 20.0f;
    static constexpr float ARENA_H = 11.25f;
    static constexpr float PIT_Y = 9.90f;

    System::ArenaBounds m_bounds;
    int m_gemsCollected = 0;

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
    void spawnGem(float x, float y);
    void spawnEnemy();
    void spawnHUD();
    void updateHUD();
    void resetGame();

    bool onMessage(const SceneMessage& message) override;
};
