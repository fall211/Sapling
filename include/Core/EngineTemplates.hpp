//
//  EngineTemplates.hpp
//  SaplingEngine
//

#pragma once

#include "Core/Engine.hpp"
#include "Core/Scene.hpp"
#include "Core/SceneMessage.hpp"
#include "Utility/Debug.hpp"

template <typename T>
bool Engine::sendToCurrentScene(const std::string& type, const T& data)
{
    if (!m_currentScene)
    {
        Debug::log("ERROR: no current scene to send message to");
        return false;
    }
    
    SceneMessage message(type, data);
    return m_currentScene->onMessage(message);
}

template <typename T>
bool Engine::sendToScene(const std::string& sceneName, const std::string& type, const T& data)
{
    auto sceneIt = m_scenes.find(sceneName);
    if (sceneIt == m_scenes.end())
    {
        Debug::log("ERROR: scene not found: " + sceneName);
        return false;
    }
    
    SceneMessage message(type, data);
    return sceneIt->second->onMessage(message);
}