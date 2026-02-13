//
//  main.cpp
//  Sapling TechDemo
//
//  A top-down 2D collector game showcasing the Sapling Engine's features:
//  - Multiple scenes (Title, Game, Score)
//  - Input system with actions
//  - Sprite rendering (static and animated)
//  - Text rendering with fonts
//  - Audio playback (music and SFX)
//  - Custom components and systems
//  - Entity-Component-System architecture
//  - Inter-scene messaging
//

#include "Core/Logger.hpp"
#include "Core/Engine.hpp"
#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Core/Input.hpp"
#include "Renderer/Sprout.hpp"

#include "TitleScene.hpp"
#include "GameScene.hpp"
#include "ScoreScene.hpp"

#include <memory>
#include <string>

int main()
{
    // ── Viewport setup ──────────────────────────────────
    const int viewportWidth = 640;
    const int viewportHeight = 480;

    // ── Initialize engine subsystems ────────────────────
    Logger::initialize();
    const auto engine = std::make_shared<Engine>(viewportWidth, viewportHeight, "Sapling TechDemo");
    AudioEngine::initialize();
    AssetManager::initialize();
    Input::initialize();

    // ── Register textures ───────────────────────────────
    // Player sprites
    AssetManager::addTexture("player", "Sprites/player.png");
    AssetManager::addTexture("player_walk", "Sprites/player_walk.png", 4);

    // Collectible sprites
    AssetManager::addTexture("gem", "Sprites/gem.png");
    AssetManager::addTexture("gem_spin", "Sprites/gem_spin.png", 4);

    // Enemy sprites
    AssetManager::addTexture("enemy", "Sprites/enemy.png");
    AssetManager::addTexture("enemy_walk", "Sprites/enemy_walk.png", 4);

    // Environment sprites
    AssetManager::addTexture("wall", "Sprites/wall.png");
    AssetManager::addTexture("floor", "Sprites/floor.png");

    // UI sprites
    AssetManager::addTexture("heart", "Sprites/heart.png");
    AssetManager::addTexture("particle", "Sprites/particle.png");

    // Debug sprite
    AssetManager::addTexture("debugCircle", "Sprites/debug.png");

    // ── Register fonts ──────────────────────────────────
    AssetManager::addFont("game_font", "Fonts/game_font.ttf", 63);

    // ── Register sounds ─────────────────────────────────
    AssetManager::addSound("bgm", "Audio/bgm.wav");
    AssetManager::addSound("collect", "Audio/collect.wav");
    AssetManager::addSound("step", "Audio/step.wav");
    AssetManager::addSound("hurt", "Audio/hurt.wav");
    AssetManager::addSound("scene_change", "Audio/scene_change.wav");
    AssetManager::addSound("score_jingle", "Audio/score_jingle.wav");

    // ── Register input actions ──────────────────────────
    Input::makeAction("moveLeft",  {SAPP_KEYCODE_LEFT,  SAPP_KEYCODE_A});
    Input::makeAction("moveRight", {SAPP_KEYCODE_RIGHT, SAPP_KEYCODE_D});
    Input::makeAction("moveUp",    {SAPP_KEYCODE_UP,    SAPP_KEYCODE_W});
    Input::makeAction("moveDown",  {SAPP_KEYCODE_DOWN,  SAPP_KEYCODE_S});
    Input::makeAction("confirm",   {SAPP_KEYCODE_SPACE, SAPP_KEYCODE_ENTER});
    Input::makeAction("quit",      {SAPP_KEYCODE_ESCAPE});

    // ── Create scenes ───────────────────────────────────
    engine->newScene<TitleScene>("title");
    engine->newScene<GameScene>("game");
    engine->newScene<ScoreScene>("score");

    // ── Start on the title screen ───────────────────────
    engine->changeScene("title");

    // ── Run the engine (blocks until window closes) ─────
    engine->run();

    // ── Cleanup ─────────────────────────────────────────
    AudioEngine::cleanUp();
    AssetManager::cleanUp();
    Input::cleanUp();
    Logger::cleanUp();

    return 0;
}

#ifdef _WIN32
#ifndef DEBUG
#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return main();
}

#endif
#endif
