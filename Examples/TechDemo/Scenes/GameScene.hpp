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
