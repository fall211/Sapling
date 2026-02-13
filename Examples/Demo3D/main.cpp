//
//  main.cpp
//  Sapling Demo3D
//
//  A 3D demo showcasing the unified 2D/3D rendering pipeline:
//  - Multi-light (directional + point)
//  - Normal mapping + roughness maps
//  - Frustum culling with BSphere
//  - worldToScreen for HUD labels tracking 3D objects
//  - .mat material file loading
//  - Unified texture registration API
//  - Unified render() call
//

#include "Core/Logger.hpp"
#include "Core/Engine.hpp"
#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Core/Input.hpp"

#include "Demo3DScene.hpp"

#include <memory>

int main()
{
    // ── Initialize engine subsystems ────────────────────
    Logger::initialize();
    const auto engine = std::make_shared<Engine>(1280, 720, "Sapling 3D Demo");
    AudioEngine::initialize();
    AssetManager::initialize();
    Input::initialize();

    // TEST 4C: Use registerTexture (unified API) instead of addTexture
    AssetManager::registerTexture("particle", "Sprites/particle.png", Sprout::TextureMode::Atlas);

    // Font for HUD text
    AssetManager::addFont("ui_font", "Fonts/game_font.ttf", 63);

    // ── Register 3D animation assets ─────────────────────
    AssetManager::addSkeleton("male_skeleton", "Models/male/Standard Walk.fbx");
    AssetManager::addAnimationClip("walk", "Models/male/Standard Walk.fbx", "male_skeleton");
    AssetManager::addAnimationClip("kneel", "Models/male/Kneeling Idle.fbx", "male_skeleton");
    AssetManager::addSkinnedMesh("male_mesh", "Models/male/Standard Walk.fbx", "male_skeleton");

    // TEST 4C: Verify findTexture works across both atlas and independent maps
    auto particleTex = AssetManager::findTexture("particle");
    if (particleTex) {
        Logger::info("TEST 4C PASS: findTexture found atlas texture 'particle'");
    } else {
        Logger::error("TEST 4C FAIL: findTexture could not find 'particle'");
    }

    // ── Register input actions ──────────────────────────
    Input::makeAction("moveLeft",  {SAPP_KEYCODE_LEFT,  SAPP_KEYCODE_A});
    Input::makeAction("moveRight", {SAPP_KEYCODE_RIGHT, SAPP_KEYCODE_D});
    Input::makeAction("moveUp",    {SAPP_KEYCODE_UP,    SAPP_KEYCODE_W});
    Input::makeAction("moveDown",  {SAPP_KEYCODE_DOWN,  SAPP_KEYCODE_S});
    Input::makeAction("quit",      {SAPP_KEYCODE_ESCAPE});
    Input::makeAction("anim1",     {SAPP_KEYCODE_1});
    Input::makeAction("anim2",     {SAPP_KEYCODE_2});

    // ── Create and start scene ──────────────────────────
    engine->newScene<Demo3DScene>("demo3d");
    engine->changeScene("demo3d");

    // ── Run (blocks until window closes) ────────────────
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
