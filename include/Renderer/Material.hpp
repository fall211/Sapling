//
//  Material.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

#include <glm/glm.hpp>
#include "sokol/sokol_gfx.h"
#include "Renderer/Texture.hpp"

#include <memory>
#include <string>

namespace Sprout
{
    enum class ShaderType
    {
        Mesh3D,
        Custom
    };

    struct MaterialProperties
    {
        glm::vec4 baseColor = glm::vec4(1.0f);
        float specularStrength = 0.5f;
        float shininess = 32.0f;
        glm::vec3 emissiveColor = glm::vec3(0.0f);
        float emissiveStrength = 0.0f;
    };

    class Material
    {
    public:
        Material() = default;
        ~Material();

        auto create(ShaderType type, const std::string& diffuse_path = "") -> bool;
        auto loadFromFile(const std::string& path) -> bool;
        auto saveToFile(const std::string& path) const -> bool;
        auto ensureLoaded() -> bool;

        auto getShaderType() const -> ShaderType { return m_shaderType; }
        auto getPipeline() -> sg_pipeline { ensureLoaded(); return m_pipeline; }
        auto getShader() -> sg_shader { ensureLoaded(); return m_shader; }

        MaterialProperties properties;
        std::shared_ptr<Texture> diffuseTexture;
        std::shared_ptr<Texture> normalTexture;
        std::shared_ptr<Texture> emissiveTexture;
        std::shared_ptr<Texture> roughnessTexture;

        void setNormalMap(const std::string& path);
        void setEmissiveMap(const std::string& path);
        void setRoughnessMap(const std::string& path);

        void release();

    private:
        ShaderType m_shaderType = ShaderType::Mesh3D;
        sg_shader m_shader = {};
        sg_pipeline m_pipeline = {};
        bool m_gpuReady = false;
        bool m_pendingCreate = false;
    };
}
