//
//  Material.cpp
//  Sapling Engine, Sprout Renderer
//

#include "Renderer/Material.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/mesh3d.h"

#include "Core/AssetManager.hpp"
#include "Core/Logger.hpp"

#include "nlohmann/json.hpp"
#include <fstream>

namespace Sprout
{

Material::~Material()
{
    release();
}

void Material::release()
{
    if (m_pipeline.id != SG_INVALID_ID) {
        sg_destroy_pipeline(m_pipeline);
        m_pipeline.id = SG_INVALID_ID;
    }
    if (m_shader.id != SG_INVALID_ID) {
        sg_destroy_shader(m_shader);
        m_shader.id = SG_INVALID_ID;
    }
    diffuseTexture.reset();
    normalTexture.reset();
    emissiveTexture.reset();
    roughnessTexture.reset();
    m_gpuReady = false;
    m_pendingCreate = false;
}

auto Material::create(ShaderType type, const std::string& diffuse_path) -> bool
{
    if (!diffuse_path.empty()) {
        diffuseTexture = AssetManager::ensureImageTexture(diffuse_path, diffuse_path);
    }
    m_shaderType = type;
    m_pendingCreate = true;
    m_gpuReady = false;
    return true;
}

auto Material::loadFromFile(const std::string& path) -> bool
{
    std::string fullPath = AssetManager::getAssetsPath() + path;
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        Logger::error("Failed to open material file: " + fullPath);
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        Logger::error("Failed to parse material file: " + fullPath + " - " + e.what());
        return false;
    }

    // shader type
    std::string shaderStr = j.value("shader", "Mesh3D");
    ShaderType type = ShaderType::Mesh3D;
    if (shaderStr == "Mesh3DSkinned") type = ShaderType::Mesh3D;

    // base color
    if (j.contains("baseColor") && j["baseColor"].is_array()) {
        auto& c = j["baseColor"];
        properties.baseColor = glm::vec4(c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>());
    }

    properties.specularStrength = j.value("specularStrength", 0.5f);
    properties.shininess = j.value("shininess", 32.0f);
    properties.emissiveStrength = j.value("emissiveStrength", 0.0f);

    if (j.contains("emissiveColor") && j["emissiveColor"].is_array()) {
        auto& c = j["emissiveColor"];
        properties.emissiveColor = glm::vec3(c[0].get<float>(), c[1].get<float>(), c[2].get<float>());
    }

    // textures
    std::string diffusePath = j.value("diffuseTexture", std::string(""));
    create(type, diffusePath);

    std::string normalPath = j.value("normalTexture", std::string(""));
    if (!normalPath.empty()) setNormalMap(normalPath);

    std::string emissivePath = j.value("emissiveTexture", std::string(""));
    if (!emissivePath.empty()) setEmissiveMap(emissivePath);

    std::string roughnessPath = j.value("roughnessTexture", std::string(""));
    if (!roughnessPath.empty()) setRoughnessMap(roughnessPath);

    return true;
}

auto Material::saveToFile(const std::string& path) const -> bool
{
    nlohmann::json j;

    switch (m_shaderType) {
        case ShaderType::Mesh3D: j["shader"] = "Mesh3D"; break;
        case ShaderType::Custom: j["shader"] = "Custom"; break;
    }

    j["baseColor"] = { properties.baseColor.r, properties.baseColor.g, properties.baseColor.b, properties.baseColor.a };
    j["specularStrength"] = properties.specularStrength;
    j["shininess"] = properties.shininess;
    j["emissiveStrength"] = properties.emissiveStrength;
    j["emissiveColor"] = { properties.emissiveColor.r, properties.emissiveColor.g, properties.emissiveColor.b };

    // texture paths — look up via AssetManager reverse maps
    auto texPath = [](const std::shared_ptr<Texture>& tex) -> std::string {
        if (!tex) return "";
        std::string name = AssetManager::getImageTextureName(tex);
        if (name.empty()) return "";
        return AssetManager::getImageTexturePath(name);
    };

    j["diffuseTexture"] = texPath(diffuseTexture);
    j["normalTexture"] = texPath(normalTexture);
    j["emissiveTexture"] = texPath(emissiveTexture);
    j["roughnessTexture"] = texPath(roughnessTexture);

    std::string fullPath = AssetManager::getAssetsPath() + path;
    std::ofstream file(fullPath);
    if (!file.is_open()) {
        Logger::error("Failed to write material file: " + fullPath);
        return false;
    }

    file << j.dump(4);
    return true;
}

auto Material::ensureLoaded() -> bool
{
    if (m_gpuReady) return true;
    if (!m_pendingCreate) return false;

    switch (m_shaderType) {
        case ShaderType::Mesh3D: {
            m_shader = sg_make_shader(mesh3d_shader_desc(sg_query_backend()));

            sg_pipeline_desc pip_desc = {};
            pip_desc.shader = m_shader;
            pip_desc.index_type = SG_INDEXTYPE_UINT32;

            // layout: vec3 position, vec3 normal, vec2 texcoord, vec4 tangent
            pip_desc.layout.attrs[ATTR_mesh3d_position0].format = SG_VERTEXFORMAT_FLOAT3;
            pip_desc.layout.attrs[ATTR_mesh3d_normal0].format = SG_VERTEXFORMAT_FLOAT3;
            pip_desc.layout.attrs[ATTR_mesh3d_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
            pip_desc.layout.attrs[ATTR_mesh3d_tangent0].format = SG_VERTEXFORMAT_FLOAT4;

            // depth test
            pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
            pip_desc.depth.write_enabled = true;

            // backface culling (use CCW for front faces)
            pip_desc.face_winding = SG_FACEWINDING_CCW;
            pip_desc.cull_mode = SG_CULLMODE_BACK;

            // alpha blending
            sg_blend_state blend = {};
            blend.enabled = true;
            blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
            blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend.op_rgb = SG_BLENDOP_ADD;
            blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
            blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend.op_alpha = SG_BLENDOP_ADD;
            pip_desc.colors[0].blend = blend;

            pip_desc.label = "mesh3d-pipeline";
            m_pipeline = sg_make_pipeline(&pip_desc);
            break;
        }
        case ShaderType::Custom:
            Logger::error("Custom shader type not yet implemented");
            return false;
    }

    if (diffuseTexture && !diffuseTexture->ensureLoaded()) {
        Logger::error("Failed to ensure diffuse texture loaded");
        return false;
    }
    if (normalTexture) normalTexture->ensureLoaded();
    if (emissiveTexture) emissiveTexture->ensureLoaded();
    if (roughnessTexture) roughnessTexture->ensureLoaded();
    m_gpuReady = true;
    m_pendingCreate = false;
    return true;
}

void Material::setNormalMap(const std::string& path)
{
    if (!path.empty()) {
        normalTexture = AssetManager::ensureImageTexture(path, path);
    }
}

void Material::setEmissiveMap(const std::string& path)
{
    if (!path.empty()) {
        emissiveTexture = AssetManager::ensureImageTexture(path, path);
    }
}

void Material::setRoughnessMap(const std::string& path)
{
    if (!path.empty()) {
        roughnessTexture = AssetManager::ensureImageTexture(path, path);
    }
}

} // namespace Sprout
