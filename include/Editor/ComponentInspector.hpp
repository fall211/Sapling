//
//  ComponentInspector.hpp
//  SaplingEngine, Editor
//

#pragma once

#ifdef SAPLING_HAS_EDITOR

#include <memory>
#include <string>
#include <nlohmann/json_fwd.hpp>

class Entity;

struct InspectorResult
{
    bool modified = false;
    std::string removeComponent;
};

class ComponentInspector
{
public:
    static InspectorResult renderAllComponents(const std::shared_ptr<Entity>& entity);

    static void serializeAllComponents(const std::shared_ptr<Entity>& entity, nlohmann::json& outComponents);
};

#endif // SAPLING_HAS_EDITOR
