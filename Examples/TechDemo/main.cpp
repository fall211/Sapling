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
    const int viewportWidth = 320;
    const int viewportHeight = 180;

    // ── Initialize engine subsystems ────────────────────
    Logger::initialize();
    const auto engine = std::make_shared<Engine>(viewportWidth, viewportHeight, "Sapling TechDemo");
    AudioEngine::initialize();
    AssetManager::initialize();
    Input::initialize();

    // ── Register scene types and load data manifest ─────
    engine->registerSceneType<TitleScene>("TitleScene");
    engine->registerSceneType<GameScene>("GameScene");
    engine->registerSceneType<ScoreScene>("ScoreScene");
    engine->loadManifest("manifest.json");

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
