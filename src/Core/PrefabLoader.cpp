//
//  PrefabLoader.cpp
//  SaplingEngine
//

#include "Core/PrefabLoader.hpp"
#include "Core/AssetManager.hpp"
#include "Core/Logger.hpp"
#include "ECS/Entity.hpp"
#include "ECS/EntityManager.hpp"
#include "ECS/Component.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

PrefabLoader* PrefabLoader::s_instance = nullptr;

PrefabLoader& PrefabLoader::getInstance()
{
    if (!s_instance)
    {
        s_instance = new PrefabLoader();
    }
    return *s_instance;
}

void PrefabLoader::initialize() {
}

void PrefabLoader::registerFactory(const std::string& name, ComponentFactory factory)
{
    m_factories[name] = std::move(factory);
}

auto PrefabLoader::getRegisteredNames() const -> std::vector<std::string>
{
    std::vector<std::string> names;
    names.reserve(m_factories.size());
    for (const auto& [name, _] : m_factories)
        names.push_back(name);
    std::sort(names.begin(), names.end());
    return names;
}

auto PrefabLoader::loadFile(const std::string& path) -> nlohmann::json
{
    std::string fullPath = AssetManager::getAssetsPath() + "/" + path;
    std::ifstream file(fullPath);

    if (!file.is_open())
    {
        Logger::error("PrefabLoader: Failed to open prefab file: " + fullPath);
        return nlohmann::json::object();
    }

    try
    {
        nlohmann::json j;
        file >> j;
        return j;
    }
    catch (const nlohmann::json::exception& e)
    {
        Logger::error("PrefabLoader: JSON parse error in " + path + ": " + e.what());
        return nlohmann::json::object();
    }
}

auto PrefabLoader::createEntityFromJson(const nlohmann::json& prefabJson,
                                        EntityManager& entityManager) -> std::shared_ptr<Entity>
{
    TagList tags;
    if (prefabJson.contains("tags"))
    {
        for (const auto& tag : prefabJson["tags"])
    {
            tags.push_back(tag.get<std::string>());
        }
    }

    auto entity = entityManager.addEntity(tags);

    if (prefabJson.contains("name"))
    {
        entity->setName(prefabJson["name"].get<std::string>());
    }

    // deserialize components
    if (prefabJson.contains("components")) {
        for (auto& [componentType, componentData] : prefabJson["components"].items()) {
            auto it = m_factories.find(componentType);
            if (it != m_factories.end()) {
                it->second(entity, componentData);
            } else {
                Logger::warn("PrefabLoader: Unknown component type '" + componentType + "'");
            }
        }
    }

    // default transform for rendering if missing
    if (!entity->hasComponent<Comp::Transform>())
    {
        if (entity->hasComponent<Comp::MeshRenderer>() ||
            entity->hasComponent<Comp::Sprite>() ||
            entity->hasComponent<Comp::Image>() ||
            entity->hasComponent<Comp::Text>())
        {
            entity->addComponent<Comp::Transform>();
        }
    }

    return entity;
}
