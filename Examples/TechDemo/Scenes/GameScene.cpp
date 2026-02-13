//
//  GameScene.cpp
//  Sapling TechDemo
//

#include "GameScene.hpp"
#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Core/Input.hpp"
#include "ECS/Component.hpp"
#include "Renderer/Sprout.hpp"
#include "Utility/Color.hpp"
#include "Utility/Debug.hpp"

#include <string>
#include <cstdlib>
#include <cmath>
#include <algorithm>

GameScene::GameScene(Engine& engine) : Scene(engine)
{
    // Set arena bounds (walls are 2 tiles thick = 32px)
    m_bounds.minX = 32.0f;
    m_bounds.minY = 32.0f;
    m_bounds.maxX = ARENA_W - 32.0f;
    m_bounds.maxY = ARENA_H - 32.0f;

    m_enemyBounds = { m_bounds.minX, m_bounds.minY, m_bounds.maxX, m_bounds.maxY };

    init();
}

void GameScene::init()
{
    resetGame();
}

void GameScene::resetGame()
{
    m_entityManager->clear();

    m_gameTimer = 60.0f;
    m_totalGemsSpawned = 0;
    m_gemsCollected = 0;
    m_gemSpawnTimer = 0.0f;
    m_gemSpawnInterval = 2.5f;
    m_gameOver = false;
    m_wave = 1;
    m_waveTimer = 0.0f;

    System::DamageState::Reset();

    spawnArena();
    spawnPlayer();
    spawnHUD();
    spawnWaveEnemies();

    // Spawn a few initial gems
    for (int i = 0; i < 4; i++)
    {
        spawnGem();
    }
}

void GameScene::update()
{
    if (m_gameOver)
    {
        // Still render so the player can see the final state
        sRender(m_entityManager->getEntities());

        if (Input::isActionUp("confirm"))
        {
            // Send score to score scene via messaging
            auto& players = m_entityManager->getEntities("player");
            int finalScore = 0;
            if (!players.empty() && players.front()->hasComponent<Comp::PlayerController>())
            {
                finalScore = players.front()->getComponent<Comp::PlayerController>().score;
            }
            m_engine.sendToScene("score", "final_score", finalScore);
            AudioEngine::playSound("scene_change");
            m_engine.changeScene("score");
        }

        return;
    }

    float dt = m_engine.deltaTime();

    // ── Update game timer ───────────────────────────────
    m_gameTimer -= dt;
    if (m_gameTimer <= 0.0f)
    {
        m_gameTimer = 0.0f;
        m_gameOver = true;
        Debug::log("Time's up! Game over.");
    }

    // ── Wave progression ────────────────────────────────
    m_waveTimer += dt;
    if (m_waveTimer >= m_waveDuration)
    {
        m_waveTimer = 0.0f;
        m_wave++;
        m_gemSpawnInterval = std::max(0.8f, m_gemSpawnInterval - 0.3f);
        spawnWaveEnemies();
        Debug::log("Wave " + std::to_string(m_wave) + " started!");
    }

    // ── Run systems ─────────────────────────────────────

    // Player movement
    System::PlayerMovement(m_entityManager, dt, m_bounds);

    // Enemy AI
    System::EnemyMovement(m_entityManager, dt, m_enemyBounds);

    // Check collectibles
    auto collectResult = System::CheckCollectibles(m_entityManager);
    m_gemsCollected += collectResult.totalCollected;

    // Check enemy-player collision
    bool playerHit = System::CheckEnemyPlayerCollision(m_entityManager, dt);
    if (playerHit)
    {
        // Check if player is dead
        auto& players = m_entityManager->getEntities("player");
        if (!players.empty() && players.front()->hasComponent<Comp::PlayerController>())
        {
            auto& controller = players.front()->getComponent<Comp::PlayerController>();
            if (controller.health <= 0)
            {
                m_gameOver = true;
                Debug::log("Player died! Game over.");
            }
        }
    }

    // Float animation for gems
    System::FloatAnimator(m_entityManager, dt);

    // ── Spawn new gems periodically ─────────────────────
    m_gemSpawnTimer += dt;
    if (m_gemSpawnTimer >= m_gemSpawnInterval)
    {
        m_gemSpawnTimer = 0.0f;
        int remaining = System::CountRemainingCollectibles(m_entityManager);
        if (remaining < m_maxGems)
        {
            spawnGem();
        }
    }

    // ── Update HUD ──────────────────────────────────────
    updateHUD();

    // ── Check for quit ──────────────────────────────────
    if (Input::isActionUp("quit"))
    {
        m_engine.changeScene("title");
    }

    // ── Render ──────────────────────────────────────────
    sRender(m_entityManager->getEntities());
}

void GameScene::onSceneEnabled()
{
    AudioEngine::playSound("bgm", true, 0.25f);
    resetGame();
}

void GameScene::onSceneDisabled()
{
    AudioEngine::stopSound("bgm");
}

void GameScene::spawnArena()
{
    auto wallTex = AssetManager::getTexture("wall");
    auto floorTex = AssetManager::getTexture("floor");

    // Spawn floor tiles (2x2 tile grid to fill the arena)
    int tilesX = ARENA_W / TILE_SIZE;
    int tilesY = ARENA_H / TILE_SIZE;

    for (int ty = 0; ty < tilesY; ty++)
    {
        for (int tx = 0; tx < tilesX; tx++)
        {
            bool isWall = (tx < 2 || tx >= tilesX - 2 || ty < 2 || ty >= tilesY - 2);

            auto tile = m_entityManager->addEntity({"arena"});
            auto& transform = tile->addComponent<Comp::Transform>(
                glm::vec2(tx * TILE_SIZE, ty * TILE_SIZE)
            );
            transform.pivot = Sprout::Pivot::TOP_LEFT;

            auto& sprite = tile->addComponent<Comp::Sprite>(isWall ? wallTex : floorTex);
            sprite.setLayer(Comp::Layer::Background);
        }
    }
}

void GameScene::spawnPlayer()
{
    auto player = m_entityManager->addEntity({"player"});
    player->setName("player");

    auto& transform = player->addComponent<Comp::Transform>(
        glm::vec2(ARENA_W / 2.0f, ARENA_H / 2.0f)
    );
    transform.pivot = Sprout::Pivot::CENTER;
    transform.scale = glm::vec3(2.0f, 2.0f, 1.0f);

    auto& sprite = player->addComponent<Comp::Sprite>(AssetManager::getTexture("player"));
    sprite.setLayer(Comp::Layer::Player);

    player->addComponent<Comp::PlayerController>(120.0f);

    // Add a bounding box for potential physics-based collision
    player->addComponent<Comp::BBox>(12.0f, 12.0f);
}

void GameScene::spawnGem()
{
    // Random position within the arena (with margin from walls)
    float margin = 48.0f;
    float x = margin + static_cast<float>(rand()) / RAND_MAX * (ARENA_W - margin * 2);
    float y = margin + static_cast<float>(rand()) / RAND_MAX * (ARENA_H - margin * 2);

    auto gem = m_entityManager->addEntity({"collectible"});

    auto& transform = gem->addComponent<Comp::Transform>(glm::vec2(x, y));
    transform.pivot = Sprout::Pivot::CENTER;
    transform.scale = glm::vec3(2.0f, 2.0f, 1.0f);

    auto& sprite = gem->addComponent<Comp::Sprite>(
        AssetManager::getTexture("gem_spin"), 6.0f
    );
    sprite.setLayer(Comp::Layer::Midground);

    // Random point values: 10, 20, or 50
    int pointValues[] = {10, 10, 10, 20, 20, 50};
    int points = pointValues[rand() % 6];

    gem->addComponent<Comp::Collectible>(points, 16.0f);
    gem->addComponent<Comp::FloatAnimation>();

    m_totalGemsSpawned++;
}

void GameScene::spawnEnemy(float x, float y, Comp::EnemyAI::PatrolType type, float speed, float distance)
{
    auto enemy = m_entityManager->addEntity({"enemy"});

    auto& transform = enemy->addComponent<Comp::Transform>(glm::vec2(x, y));
    transform.pivot = Sprout::Pivot::CENTER;
    transform.scale = glm::vec3(2.0f, 2.0f, 1.0f);

    auto& sprite = enemy->addComponent<Comp::Sprite>(
        AssetManager::getTexture("enemy_walk"), 5.0f
    );
    sprite.setLayer(Comp::Layer::Midground);

    enemy->addComponent<Comp::EnemyAI>(type, speed, distance);
}

void GameScene::spawnWaveEnemies()
{
    float cx = ARENA_W / 2.0f;
    float cy = ARENA_H / 2.0f;

    switch (m_wave)
    {
        case 1:
            // Wave 1: Two horizontal patrol enemies
            spawnEnemy(120, 120, Comp::EnemyAI::PatrolType::HORIZONTAL, 40.0f, 100.0f);
            spawnEnemy(500, 360, Comp::EnemyAI::PatrolType::HORIZONTAL, 50.0f, 120.0f);
            break;

        case 2:
            // Wave 2: Add vertical patrol enemies
            spawnEnemy(200, 200, Comp::EnemyAI::PatrolType::VERTICAL, 45.0f, 150.0f);
            spawnEnemy(440, 150, Comp::EnemyAI::PatrolType::VERTICAL, 55.0f, 100.0f);
            break;

        case 3:
        default:
            // Wave 3+: Add a chase enemy plus patrol
            spawnEnemy(100, 400, Comp::EnemyAI::PatrolType::CHASE, 35.0f + m_wave * 3.0f, 200.0f);
            spawnEnemy(
                60.0f + static_cast<float>(rand() % 500),
                60.0f + static_cast<float>(rand() % 360),
                Comp::EnemyAI::PatrolType::HORIZONTAL,
                40.0f + m_wave * 5.0f,
                80.0f + m_wave * 10.0f
            );
            break;
    }
}

void GameScene::spawnHUD()
{
    // Score display (top-left)
    auto scoreText = m_entityManager->addEntity({"hud", "score_display"});
    scoreText->addComponent<Comp::Transform>(glm::vec2(-300, -10), Sprout::Pivot::TOP_CENTER, true);
    scoreText->addComponent<Comp::Text>(
        "Score: 0",
        "game_font", 12, Color::White,
        Sprout::TextJustify::LEFT
    );

    // Timer display (top-right)
    auto timerText = m_entityManager->addEntity({"hud", "timer_display"});
    timerText->addComponent<Comp::Transform>(glm::vec2(300, -10), Sprout::Pivot::TOP_CENTER, true);
    timerText->addComponent<Comp::Text>(
        "Time: 60",
        "game_font", 12, Color::White,
        Sprout::TextJustify::RIGHT
    );

    // Wave display (top-center)
    auto waveText = m_entityManager->addEntity({"hud", "wave_display"});
    waveText->addComponent<Comp::Transform>(glm::vec2(0, -10), Sprout::Pivot::TOP_CENTER, true);
    waveText->addComponent<Comp::Text>(
        "Wave 1",
        "game_font", 12, Color::Gold,
        Sprout::TextJustify::CENTER
    );

    // Health display using heart sprites (bottom-left)
    for (int i = 0; i < 3; i++)
    {
        auto heart = m_entityManager->addEntity({"hud", "heart"});
        heart->addComponent<Comp::Transform>(
            glm::vec2(-300 + i * 30, 15),
            Sprout::Pivot::BOTTOM_CENTER, true
        );
        auto& heartSprite = heart->addComponent<Comp::Sprite>(AssetManager::getTexture("heart"));
        heartSprite.setLayer(Comp::Layer::UserInterface);
    }

    // Gem count display (bottom-center)
    auto gemCount = m_entityManager->addEntity({"hud", "gem_count"});
    gemCount->addComponent<Comp::Transform>(glm::vec2(0, 15), Sprout::Pivot::BOTTOM_CENTER, true);
    gemCount->addComponent<Comp::Text>(
        "Gems: 0",
        "game_font", 10, Color::Yellow,
        Sprout::TextJustify::CENTER
    );

    // Game over text (hidden initially)
    auto gameOverText = m_entityManager->addEntity({"hud", "gameover_text"});
    gameOverText->addComponent<Comp::Transform>(glm::vec2(0, 0), Sprout::Pivot::CENTER, true);
    auto& goText = gameOverText->addComponent<Comp::Text>(
        "",
        "game_font", 24, Color::Red,
        Sprout::TextJustify::CENTER
    );
    goText.layer = Comp::Layer::UserInterface;

    // "Press Space" after game over (hidden initially)
    auto continuePrompt = m_entityManager->addEntity({"hud", "continue_prompt"});
    continuePrompt->addComponent<Comp::Transform>(glm::vec2(0, -40), Sprout::Pivot::CENTER, true);
    auto& cpText = continuePrompt->addComponent<Comp::Text>(
        "",
        "game_font", 12, Color::White,
        Sprout::TextJustify::CENTER
    );
    cpText.layer = Comp::Layer::UserInterface;
}

void GameScene::updateHUD()
{
    // Get player score and health
    int score = 0;
    int health = 3;
    auto& players = m_entityManager->getEntities("player");
    if (!players.empty() && players.front()->hasComponent<Comp::PlayerController>())
    {
        auto& controller = players.front()->getComponent<Comp::PlayerController>();
        score = controller.score;
        health = controller.health;
    }

    // Update score text
    auto& scoreEntities = m_entityManager->getEntities("score_display");
    if (!scoreEntities.empty() && scoreEntities.front()->hasComponent<Comp::Text>())
    {
        scoreEntities.front()->getComponent<Comp::Text>().text = "Score: " + std::to_string(score);
    }

    // Update timer text
    auto& timerEntities = m_entityManager->getEntities("timer_display");
    if (!timerEntities.empty() && timerEntities.front()->hasComponent<Comp::Text>())
    {
        int timeLeft = static_cast<int>(std::ceil(m_gameTimer));
        auto& timerText = timerEntities.front()->getComponent<Comp::Text>();
        timerText.text = "Time: " + std::to_string(timeLeft);

        // Flash red when time is low
        if (timeLeft <= 10)
        {
            timerText.color = (static_cast<int>(m_gameTimer * 4) % 2 == 0)
                ? Color::Red
                : Color::White;
        }
        else
        {
            timerText.color = Color::White;
        }
    }

    // Update wave text
    auto& waveEntities = m_entityManager->getEntities("wave_display");
    if (!waveEntities.empty() && waveEntities.front()->hasComponent<Comp::Text>())
    {
        waveEntities.front()->getComponent<Comp::Text>().text = "Wave " + std::to_string(m_wave);
    }

    // Update hearts (hide hearts when health is lost)
    auto& hearts = m_entityManager->getEntities("heart");
    int heartIdx = 0;
    for (auto& heart : hearts)
    {
        if (heart->hasComponent<Comp::Sprite>())
        {
            heart->getComponent<Comp::Sprite>().enabled = (heartIdx < health);
        }
        heartIdx++;
    }

    // Update gem count
    auto& gemCountEntities = m_entityManager->getEntities("gem_count");
    if (!gemCountEntities.empty() && gemCountEntities.front()->hasComponent<Comp::Text>())
    {
        gemCountEntities.front()->getComponent<Comp::Text>().text = "Gems: " + std::to_string(m_gemsCollected);
    }

    // Update game over text
    if (m_gameOver)
    {
        auto& goEntities = m_entityManager->getEntities("gameover_text");
        if (!goEntities.empty() && goEntities.front()->hasComponent<Comp::Text>())
        {
            auto& goText = goEntities.front()->getComponent<Comp::Text>();
            if (health <= 0)
            {
                goText.text = "GAME OVER";
                goText.color = Color::Red;
            }
            else
            {
                goText.text = "TIME UP!";
                goText.color = Color::Gold;
            }
        }

        auto& continueEntities = m_entityManager->getEntities("continue_prompt");
        if (!continueEntities.empty() && continueEntities.front()->hasComponent<Comp::Text>())
        {
            continueEntities.front()->getComponent<Comp::Text>().text = "Press SPACE to continue";
        }
    }
}

bool GameScene::onMessage(const SceneMessage& message)
{
    if (message.hasType("restart"))
    {
        resetGame();
        return true;
    }

    return false;
}