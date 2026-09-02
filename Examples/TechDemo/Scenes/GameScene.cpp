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
    spawnArena();
    spawnPlayer();
    spawnHUD();
}

void GameScene::update()
{
    float dt = m_engine.deltaTime();
    System::PlayerMovement(m_entityManager, dt, m_bounds);

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
    spawnPlatform(14, 18, 8);
}

void GameScene::spawnPlayer()
{
    auto player = m_entityManager->addEntity({"player"});
    player->setName("player");

    const float halfH = 0.375f;
    const float startTop = 10.0f * TILE_SIZE;
    auto& transform = player->addComponent<Comp::Transform>(
        glm::vec2(2.5f, startTop - halfH)
    );
    transform.pivot = Sprout::Pivot::CENTER;
    transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

    auto& sprite = player->addComponent<Comp::Sprite>(AssetManager::getTexture("player"));
    sprite.setLayer(Comp::Layer::Player);

    player->addComponent<Comp::PlayerController>(5.0f);
    player->addComponent<Comp::BBox>(0.75f, 0.75f);
}

void GameScene::spawnHUD()
{
    auto hint = m_entityManager->addEntity({"hud", "hint"});
    hint->addComponent<Comp::Transform>(glm::vec2(0, 24), Sprout::Pivot::TOP_CENTER, true);
    hint->addComponent<Comp::Text>(
        "A/D walk   SPACE jump",
        "game_font", 12, Color::White,
        Sprout::TextJustify::CENTER
    );
}

bool GameScene::onMessage(const SceneMessage& /*message*/)
{
    return false;
}
