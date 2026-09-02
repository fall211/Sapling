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

#include "glm/geometric.hpp"

#include <string>

GameScene::GameScene(Engine& engine) : Scene(engine)
{
    m_bounds.minX = 1.0f;
    m_bounds.minY = 1.0f;
    m_bounds.maxX = ARENA_W - 1.0f;
    m_bounds.maxY = ARENA_H - 1.0f;

    init();
}

void GameScene::init()
{
    resetGame();
}

void GameScene::resetGame()
{
    m_entityManager->clear();
    m_gemsCollected = 0;
    m_failTimer = 0.0f;
    System::DamageState::Reset();
    spawnArena();
    spawnPlayer();
    spawnGem(4.6f, 9.15f);
    spawnGem(9.0f, 8.15f);
    spawnGem(1.90f, 9.15f);
    spawnEnemy();
    spawnExit();
    spawnHUD();
}

void GameScene::update()
{
    float dt = m_engine.deltaTime();

    if (m_failTimer > 0.0f)
    {
        m_failTimer -= dt;
        if (Input::isActionUp("quit"))
        {
            m_engine.changeScene("title");
            return;
        }
        sRender(m_entityManager->getEntities());
        if (m_failTimer <= 0.0f)
        {
            resetGame();
        }
        return;
    }

    System::PlayerMovement(m_entityManager, dt, m_bounds);
    System::EnemyMovement(m_entityManager, dt, System::EnemyAIBounds{1.0f, 9.90f, 19.0f, 12.0f});
    System::FloatMotion(m_entityManager, dt);

    auto collected = System::CheckCollectibles(m_entityManager);
    if (collected.totalCollected > 0)
    {
        m_gemsCollected += collected.totalCollected;
        updateHUD();
    }

    bool hit = System::CheckEnemyPlayerCollision(m_entityManager, dt);
    bool pit = false;
    glm::vec2 playerPos(0.0f);
    auto& players = m_entityManager->getEntities("player");
    if (!players.empty() && players.front()->hasComponent<Comp::Transform>())
    {
        playerPos = glm::vec2(players.front()->getComponent<Comp::Transform>().position);
        pit = players.front()->getComponent<Comp::Transform>().position.y > PIT_Y;
    }
    if (pit)
    {
        beginFail("YOU FELL", 0.85f);
        sRender(m_entityManager->getEntities());
        return;
    }
    if (hit)
    {
        beginFail(nullptr, 0.70f);
        sRender(m_entityManager->getEntities());
        return;
    }

    auto& exits = m_entityManager->getEntities("exit");
    if (!exits.empty() && exits.front()->hasComponent<Comp::Transform>())
    {
        glm::vec2 exitPos = glm::vec2(exits.front()->getComponent<Comp::Transform>().position);
        if (glm::distance(playerPos, exitPos) < 0.7f)
        {
            if (m_gemsCollected >= 3)
            {
                AudioEngine::playSound("scene_change");
                m_engine.changeScene("score");
                sRender(m_entityManager->getEntities());
                return;
            }
            updateHUD();
            auto& hints = m_entityManager->getEntities("hint");
            if (!hints.empty() && hints.front()->hasComponent<Comp::Text>())
            {
                hints.front()->getComponent<Comp::Text>().text = "Need 3 gems";
                hints.front()->getComponent<Comp::Text>().color = Color::Gold;
            }
        }
    }

    if (Input::isActionUp("quit"))
    {
        m_engine.changeScene("title");
    }

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

void GameScene::spawnSolidTile(int tx, int ty, bool wall)
{
    auto tile = m_entityManager->addEntity({"arena", "solid", wall ? "wall" : "platform"});
    auto& transform = tile->addComponent<Comp::Transform>(
        glm::vec2(static_cast<float>(tx) * TILE_SIZE, static_cast<float>(ty) * TILE_SIZE)
    );
    transform.pivot = Sprout::Pivot::TOP_LEFT;
    auto tex = AssetManager::getTexture(wall ? "wall" : "floor");
    auto& sprite = tile->addComponent<Comp::Sprite>(tex);
    sprite.setLayer(wall ? Comp::Layer::Background : Comp::Layer::Midground);
    tile->addComponent<Comp::BBox>(TILE_SIZE, TILE_SIZE);
}

void GameScene::spawnPlatform(int tx0, int tx1, int ty)
{
    for (int tx = tx0; tx <= tx1; tx++)
    {
        spawnSolidTile(tx, ty, false);
    }
}

void GameScene::spawnArena()
{
    for (int ty = 0; ty < ARENA_ROWS; ty++)
    {
        for (int tx = 0; tx < ARENA_COLS; tx++)
        {
            bool isWall = (tx < 1 || tx >= ARENA_COLS - 1 || ty < 1 || ty >= ARENA_ROWS - 1);
            if (isWall)
            {
                spawnSolidTile(tx, ty, true);
            }
        }
    }

    spawnPlatform(1, 5, 10);
    spawnPlatform(7, 11, 9);
    spawnPlatform(12, 18, 9);
}

void GameScene::spawnPlayer()
{
    auto player = m_entityManager->addEntity({"player"});
    player->setName("player");

    const float halfH = 0.375f;
    const float startTop = 10.0f * TILE_SIZE;
    auto& transform = player->addComponent<Comp::Transform>(
        glm::vec2(3.4f, startTop - halfH)
    );
    transform.pivot = Sprout::Pivot::CENTER;
    transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

    auto& sprite = player->addComponent<Comp::Sprite>(AssetManager::getTexture("player"));
    sprite.setLayer(Comp::Layer::Player);

    player->addComponent<Comp::PlayerController>(5.0f);
    player->addComponent<Comp::BBox>(0.75f, 0.75f);
}

void GameScene::spawnGem(float x, float y)
{
    auto gem = m_entityManager->addEntity({"collectible"});
    auto& transform = gem->addComponent<Comp::Transform>(glm::vec2(x, y));
    transform.pivot = Sprout::Pivot::CENTER;
    auto& sprite = gem->addComponent<Comp::Sprite>(AssetManager::getTexture("gem_spin"), 6.0f);
    sprite.setLayer(Comp::Layer::Midground);
    gem->addComponent<Comp::Collectible>(10, 0.875f);
    gem->addComponent<Comp::FloatMotion>(2.0f, 0.08f);
}

void GameScene::spawnEnemy()
{
    auto enemy = m_entityManager->addEntity({"enemy"});
    enemy->setName("enemy");
    auto& transform = enemy->addComponent<Comp::Transform>(glm::vec2(6.50f, 11.15f));
    transform.pivot = Sprout::Pivot::CENTER;
    auto& sprite = enemy->addComponent<Comp::Sprite>(AssetManager::getTexture("enemy_walk"), 5.0f);
    sprite.setLayer(Comp::Layer::Midground);
    auto& ai = enemy->addComponent<Comp::EnemyAI>(Comp::EnemyAI::PatrolType::HORIZONTAL, 0.5f, 0.20f);
    ai.direction = glm::vec2(-1.0f, 0.0f);
}

void GameScene::spawnExit()
{
    auto exit = m_entityManager->addEntity({"exit"});
    exit->setName("exit");
    auto& transform = exit->addComponent<Comp::Transform>(glm::vec2(17.6f, 8.15f));
    transform.pivot = Sprout::Pivot::CENTER;
    transform.scale = glm::vec3(1.25f, 1.25f, 1.0f);
    auto& sprite = exit->addComponent<Comp::Sprite>(AssetManager::getTexture("heart"));
    sprite.setLayer(Comp::Layer::Player);

    auto label = m_entityManager->addEntity({"exit", "exit_label"});
    auto& lt = label->addComponent<Comp::Transform>(glm::vec2(16.7f, 7.50f));
    lt.pivot = Sprout::Pivot::CENTER;
    label->addComponent<Comp::Text>(
        "EXIT",
        "game_font", 2, Color::White,
        Sprout::TextJustify::CENTER
    );
}

void GameScene::spawnHUD()
{
    auto hint = m_entityManager->addEntity({"hud", "hint"});
    hint->addComponent<Comp::Transform>(glm::vec2(0, 24), Sprout::Pivot::TOP_CENTER, true);
    hint->addComponent<Comp::Text>(
        "",
        "game_font", 12, Color::White,
        Sprout::TextJustify::CENTER
    );

    auto gems = m_entityManager->addEntity({"hud", "gem_count"});
    gems->addComponent<Comp::Transform>(glm::vec2(24, 24), Sprout::Pivot::TOP_LEFT, true);
    gems->addComponent<Comp::Text>(
        "Gems: 0/3",
        "game_font", 12, Color::Yellow,
        Sprout::TextJustify::LEFT
    );
}

void GameScene::beginFail(const char* banner, float seconds)
{
    m_failTimer = seconds;
    if (banner == nullptr || banner[0] == '\0')
    {
        return;
    }
    auto fail = m_entityManager->addEntity({"hud", "fail"});
    fail->setName("fail");
    fail->addComponent<Comp::Transform>(glm::vec2(0, 72), Sprout::Pivot::TOP_CENTER, true);
    fail->addComponent<Comp::Text>(
        banner,
        "game_font", 28, Color::Red,
        Sprout::TextJustify::CENTER
    );
}

void GameScene::updateHUD()
{
    auto& gems = m_entityManager->getEntities("gem_count");
    if (!gems.empty() && gems.front()->hasComponent<Comp::Text>())
    {
        gems.front()->getComponent<Comp::Text>().text =
            "Gems: " + std::to_string(m_gemsCollected) + "/3";
    }
}

bool GameScene::onMessage(const SceneMessage& /*message*/)
{
    return false;
}
