//
//  Engine.cpp
//  SaplingEngine
//


#include "Core/Engine.hpp"
#ifdef SAPLING_AGENT_CLI
#include "Core/AgentCli.hpp"
#endif
#include "Core/AssetManager.hpp"
#include "Core/Input.hpp"
#include "Core/ManifestLoader.hpp"
#include "Core/SceneMessage.hpp"
#include "Core/Logger.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <memory>


Engine::Engine(size_t viewportWidth, size_t viewportHeight, const char* title)
    : m_window(viewportWidth, viewportHeight, title)
{
    m_scenes = sceneMap();
    AssetManager::getInstance();

    Logger::info("Engine init completed");
}

void Engine::run()
{
    m_window.SetUpdateFrameCallback([this](double dt) { this -> update(dt);});
#ifdef SAPLING_AGENT_CLI
    AgentCli::bind(*this);
    AgentCli::start();
    m_window.SetPostFrameCallback([]() { AgentCli::afterPresent(); });
#endif

    m_window.Run(); // nothing after this gets called
}

void Engine::update(double dt)
{
    m_deltaTime = dt;
#ifdef SAPLING_AGENT_CLI
    AgentCli::drain();
#endif

    if (!m_currentScene)
    {
        Logger::error("No scene set");
        return;
    }

    m_currentScene->preUpdate();
    m_currentScene->update();
    m_currentScene->postUpdate();
    m_currentFrame++;
}

void Engine::makeScene(const std::string& name, std::shared_ptr<Scene> ptr)
{
    if (m_scenes.find(name) == m_scenes.end())
    {
        m_scenes[name] = std::move(ptr);
    }
    else
    {
        Logger::warn("Scene already exists: " + name);
    }
}

void Engine::registerSceneFactory(const std::string& type, SceneFactory factory)
{
    m_sceneFactories[type] = std::move(factory);
}

void Engine::loadManifest(const std::string& manifestPath)
{
    const nlohmann::json manifestJson = ManifestLoader::loadJson(manifestPath);

    AssetManager::loadManifest(manifestJson, manifestPath);
    Input::loadManifest(manifestJson, manifestPath);

    if (!manifestJson.is_object())
    {
        return;
    }

    if (manifestJson.contains("scenes"))
    {
        if (!manifestJson["scenes"].is_array())
        {
            Logger::error("Engine: " + manifestPath + " field 'scenes' must be an array");
        }
        else
        {
            for (const auto& sceneJson : manifestJson["scenes"])
            {
                if (!sceneJson.is_object())
                {
                    Logger::error("Engine: " + manifestPath + " has a scene entry that is not an object");
                    continue;
                }

                if (!sceneJson.contains("name") || !sceneJson["name"].is_string() || sceneJson["name"].get<std::string>().empty())
                {
                    Logger::error("Engine: " + manifestPath + " has scene entry without required string field 'name'");
                    continue;
                }

                if (!sceneJson.contains("type") || !sceneJson["type"].is_string() || sceneJson["type"].get<std::string>().empty())
                {
                    Logger::error("Engine: " + manifestPath + " scene '" + sceneJson["name"].get<std::string>() + "' is missing required string field 'type'");
                    continue;
                }

                const std::string sceneName = sceneJson["name"].get<std::string>();
                const std::string sceneType = sceneJson["type"].get<std::string>();
                const auto factoryIt = m_sceneFactories.find(sceneType);
                if (factoryIt == m_sceneFactories.end())
                {
                    Logger::error("Engine: " + manifestPath + " scene '" + sceneName + "' references unregistered scene type '" + sceneType + "'");
                    continue;
                }

                makeScene(sceneName, factoryIt->second(*this));
            }
        }
    }

    if (manifestJson.contains("initialScene"))
    {
        if (!manifestJson["initialScene"].is_string())
        {
            Logger::error("Engine: " + manifestPath + " field 'initialScene' must be a string");
            return;
        }

        const std::string initialScene = manifestJson["initialScene"].get<std::string>();
        if (m_scenes.find(initialScene) == m_scenes.end())
        {
            Logger::error("Engine: " + manifestPath + " initial scene '" + initialScene + "' was not loaded");
            return;
        }

        changeScene(initialScene);
    }
}

void Engine::changeScene(const std::string& name)
{
    if (m_currentScene)
    {
        m_currentScene->disable();
    }
    m_currentScene = getScene(name);
    m_currentSceneName = name;
    m_currentScene->enable();
}

auto Engine::hasScene(const std::string& name) const -> bool
{
    return m_scenes.find(name) != m_scenes.end();
}

auto Engine::sceneNames() const -> std::vector<std::string>
{
    std::vector<std::string> names;
    names.reserve(m_scenes.size());
    for (const auto& entry : m_scenes)
    {
        names.push_back(entry.first);
    }
    return names;
}

auto Engine::getScene(const std::string& name) -> std::shared_ptr<Scene>
{
    return m_scenes[name];
}

auto Engine::getCurrentScene() -> std::shared_ptr<Scene>&
{
    return m_currentScene;
}

#include "Core/EngineT.hpp"
template bool Engine::sendToScene<int>(const std::string&, const std::string&, const int&);
template bool Engine::sendToScene<float>(const std::string&, const std::string&, const float&);
template bool Engine::sendToScene<std::string>(const std::string&, const std::string&, const std::string&);
template bool Engine::sendToScene<bool>(const std::string&, const std::string&, const bool&);

template bool Engine::sendToCurrentScene<int>(const std::string&, const int&);
template bool Engine::sendToCurrentScene<float>(const std::string&, const float&);
template bool Engine::sendToCurrentScene<std::string>(const std::string&, const std::string&);
template bool Engine::sendToCurrentScene<bool>(const std::string&, const bool&);
