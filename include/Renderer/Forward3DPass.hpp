//
//  Forward3DPass.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

#include "Renderer/RenderPass.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"

#include <glm/glm.hpp>
#include "sokol/sokol_gfx.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Sprout
{
    enum class BlendMode3D : std::uint8_t
    {
        Alpha = 0,
        Opaque = 1
    };

    struct PipelineState3D
    {
        bool depthTest = true;
        bool depthWrite = true;
        bool doubleSided = false;
        BlendMode3D blendMode = BlendMode3D::Alpha;
    };

    struct MeshDrawRequest
    {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
        glm::mat4 modelMatrix;
        PipelineState3D pipelineState;

        bool isSkinned = false;
        const std::vector<glm::mat4>* boneMatrices = nullptr;
    };

    static constexpr int MAX_LIGHTS = 8;

    struct Frustum
    {
        glm::vec4 planes[6]; // left, right, bottom, top, near, far (ax+by+cz+d form)

        void extract(const glm::mat4& vp)
        {
            // left
            planes[0] = glm::vec4(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0],
                                   vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]);
            // right
            planes[1] = glm::vec4(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0],
                                   vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]);
            // bottom
            planes[2] = glm::vec4(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1],
                                   vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]);
            // top
            planes[3] = glm::vec4(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1],
                                   vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]);
            // near
            planes[4] = glm::vec4(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2],
                                   vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]);
            // far
            planes[5] = glm::vec4(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2],
                                   vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]);

            for (auto& p : planes) {
                float len = glm::length(glm::vec3(p));
                if (len > 0.0f) p /= len;
            }
        }

        bool testSphere(const glm::vec3& center, float radius) const
        {
            for (const auto& p : planes) {
                float dist = glm::dot(glm::vec3(p), center) + p.w;
                if (dist < -radius) return false;
            }
            return true;
        }

        bool testAABB(const glm::vec3& center, const glm::vec3& halfExtents) const
        {
            for (const auto& p : planes) {
                glm::vec3 n(p);
                float r = halfExtents.x * std::abs(n.x) + halfExtents.y * std::abs(n.y) + halfExtents.z * std::abs(n.z);
                float dist = glm::dot(n, center) + p.w;
                if (dist < -r) return false;
            }
            return true;
        }
    };

    struct LightData
    {
        glm::vec3 positionOrDirection = glm::vec3(0.0f);
        float type = 0.0f; // 0 = directional, 1 = point
        glm::vec3 color = glm::vec3(1.0f);
        float range = 10.0f;
    };

    struct SceneData3D
    {
        glm::mat4 viewMatrix = glm::mat4(1.0f);
        glm::mat4 projectionMatrix = glm::mat4(1.0f);
        glm::vec3 cameraPosition = glm::vec3(0.0f);
        float ambientStrength = 0.15f;

        LightData lights[MAX_LIGHTS];
        int numLights = 0;
    };

    class Forward3DPass : public RenderPass
    {
    public:
        Forward3DPass() = default;
        ~Forward3DPass() override = default;

        void submit(const std::shared_ptr<Mesh>& mesh,
                    const std::shared_ptr<Material>& material,
                    const glm::mat4& modelMatrix,
                    const PipelineState3D& pipelineState = {},
                    bool isSkinned = false,
                    const std::vector<glm::mat4>* boneMatrices = nullptr);

        void execute(Window& window) override;
        void clear() override;

        auto hasRequests() const -> bool { return !m_drawRequests.empty(); }
        auto getDrawRequestCount() const -> size_t { return m_drawRequests.size(); }

        SceneData3D sceneData;
        Frustum frustum;

    private:
        std::vector<MeshDrawRequest> m_drawRequests;

        // default textures for meshes without specific maps
        sg_image m_whiteTex = {};
        sg_image m_flatNormalTex = {};
        sg_image m_blackTex = {};
        sg_image m_midGrayTex = {};
        sg_sampler m_sampler = {};
        bool m_resourcesCreated = false;
        std::unordered_map<uint64_t, sg_pipeline> m_pipelineVariants;

        void ensureResources();
        auto getPipelineForRequest(const MeshDrawRequest& request) -> sg_pipeline;
        auto makePipelineVariant(bool isSkinned, const PipelineState3D& state) -> sg_pipeline;
    };
}
