//
//  Engine.cpp
//  SaplingEngine
//


#include "Core/Engine.hpp"
#include "Core/AssetManager.hpp"
#include "Core/SceneMessage.hpp"

#include <cstddef>
#include <memory>


Engine::Engine(size_t viewportWidth, size_t viewportHeight, const char* title)
    : m_window(viewportWidth, viewportHeight, title)
{
    m_scenes = sceneMap();
    AssetManager::getInstance();

    Debug::log("init completed");
}

void Engine::run()
{
    m_window.SetUpdateFrameCallback([this](double dt) { this -> update(dt);});
    
    m_window.Run(); // nothing after this gets called
}

void Engine::update(double dt)
{
    m_deltaTime = dt;
    
    if (!m_currentScene)
    {
        Debug::log("ERR: No scene set");
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
        Debug::log("scene already exists");
    }
}

void Engine::changeScene(const std::string& name)
{
    if (m_currentScene)
    {
        m_currentScene->disable();
    }
    m_currentScene = getScene(name);
    m_currentScene->enable();
}

auto Engine::getScene(const std::string& name) -> std::shared_ptr<Scene>
{
    return m_scenes[name];
}

auto Engine::getCurrentScene() -> std::shared_ptr<Scene>&
{
    return m_currentScene;
}

#include "Core/EngineTemplates.hpp"
template bool Engine::sendToScene<int>(const std::string&, const std::string&, const int&);
template bool Engine::sendToScene<float>(const std::string&, const std::string&, const float&);
template bool Engine::sendToScene<std::string>(const std::string&, const std::string&, const std::string&);
template bool Engine::sendToScene<bool>(const std::string&, const std::string&, const bool&);

template bool Engine::sendToCurrentScene<int>(const std::string&, const int&);
template bool Engine::sendToCurrentScene<float>(const std::string&, const float&);
template bool Engine::sendToCurrentScene<std::string>(const std::string&, const std::string&);
template bool Engine::sendToCurrentScene<bool>(const std::string&, const bool&);

