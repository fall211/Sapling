//
//  PrefabLoader.hpp
//  SaplingEngine
//

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json_fwd.hpp>

class Entity;
class EntityManager;

class PrefabLoader
{
public:
    using ComponentFactory = std::function<void(std::shared_ptr<Entity>, const nlohmann::json&)>;

    static PrefabLoader& getInstance();

    void initialize();

    template <typename T>
    static void registerComponent(const std::string& name);

    void registerFactory(const std::string& name, ComponentFactory factory);

    auto getRegisteredNames() const -> std::vector<std::string>;

    auto loadFile(const std::string& path) -> nlohmann::json;

    auto createEntityFromJson(const nlohmann::json& prefabJson,
                              EntityManager& entityManager) -> std::shared_ptr<Entity>;

private:
    PrefabLoader() = default;

    std::unordered_map<std::string, ComponentFactory> m_factories;
    static PrefabLoader* s_instance;
};

#include "PrefabLoaderT.hpp"
