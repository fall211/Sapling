//
//  main.cpp
//  Sapling Prefab Editor
//

#include "Core/Logger.hpp"
#include "Core/Engine.hpp"
#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Core/Input.hpp"
#include "Editor/PrefabEditor.hpp"

#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[])
{
    // ── Parse CLI args ──
    std::string assetsPath;
    if (argc >= 2) {
        assetsPath = argv[1];
        if (!assetsPath.empty() && assetsPath.back() != '/')
            assetsPath += '/';
    } else {
        std::cerr << "Usage: SaplingEditor <path/to/project/Assets>" << '\n';
        return 1;
    }

    // ── Initialize subsystems ──
    Logger::initialize();
    auto engine = std::make_shared<Engine>(1280, 720, "Sapling Prefab Editor");
    AudioEngine::initialize();
    AssetManager::initialize();
    Input::initialize();

    AssetManager::setAssetsPath(assetsPath);

    // ── Create editor scene and run ──
    engine->newScene<PrefabEditor>("editor");
    engine->changeScene("editor");
    engine->run();

    // ── Cleanup ──
    AudioEngine::cleanUp();
    AssetManager::cleanUp();
    Input::cleanUp();
    Logger::cleanUp();

    return 0;
}
