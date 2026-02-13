//
//  PrefabLoaderT.hpp
//  SaplingEngine — template implementations for PrefabLoader
//

#pragma once

#include "Core/PrefabLoader.hpp"
#include "ECS/Entity.hpp"
#include <nlohmann/json.hpp>

template <typename T>
void PrefabLoader::registerComponent(const std::string& name) {
    getInstance().registerFactory(name, [](const std::shared_ptr<Entity>& entity, const nlohmann::json& data) {
        auto& comp = entity->addComponent<T>();
        comp.deserialize(data);
    });
}
