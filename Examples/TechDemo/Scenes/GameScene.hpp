//
//  GameScene.hpp
//  Sapling TechDemo
//

#pragma once

#include "Core/Scene.hpp"
#include "Utility/Color.hpp"

#include "sPlayerMovement.hpp"
#include "sCollectibles.hpp"
#include "sFloatAnimator.hpp"
#include "sEnemyAI.hpp"

#include "PlayerController.hpp"
#include "Collectible.hpp"
#include "FloatAnimation.hpp"
#include "EnemyAI.hpp"

class GameScene : public Scene
{
private:
    // Arena dimensions (in pixels)
    static constexpr int ARENA_COLS = 20;
    static constexpr int ARENA_ROWS = 15;
    static constexpr int TILE_SIZE = 16;
    static constexpr int ARENA_W = ARENA_COLS * TILE_SIZE * 2;  // 640
    static constexpr int ARENA_H = ARENA_ROWS * TILE_SIZE * 2;  // 480

    System::ArenaBounds m_bounds;
    System::EnemyAIBounds m_enemyBounds;

    // Game state
    float m_gameTimer = 60.0f;       // seconds remaining
    int m_totalGemsSpawned = 0;
    int m_gemsCollected = 0;
    float m_gemSpawnTimer = 0.0f;
    float m_gemSpawnInterval = 2.5f; // seconds between gem spawns
    int m_maxGems = 8;               // max gems on screen at once
    bool m_gameOver = false;
    int m_wave = 1;
    float m_waveTimer = 0.0f;
    float m_waveDuration = 20.0f;

public:
    GameScene(Engine& engine);
    ~GameScene() = default;

    void init() override;
    void update() override;
    void onSceneEnabled() override;
    void onSceneDisabled() override;

    void spawnArena();
    void spawnPlayer();
    void spawnGem();
    void spawnEnemy(float x, float y, Comp::EnemyAI::PatrolType type, float speed, float distance);
    void spawnWaveEnemies();
    void spawnHUD();
    void updateHUD();
    void resetGame();

    bool onMessage(const SceneMessage& message) override;
};