//
//  Mesh.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

#include <glm/glm.hpp>
#include "sokol/sokol_gfx.h"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace Sprout
{
    class Skeleton;
    struct Mesh3DVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        glm::vec4 tangent;
    };

    struct Mesh3DVertexSkinned
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        glm::vec4 tangent;
        glm::vec4 boneIndices;
        glm::vec4 boneWeights;
    };

    void computeTangents(std::vector<Mesh3DVertex>& vertices, const std::vector<uint32_t>& indices);
    void computeTangentsSkinned(std::vector<Mesh3DVertexSkinned>& vertices, const std::vector<uint32_t>& indices);

    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh();

        auto loadOBJ(const std::string& path) -> bool;
        auto loadFBX(const std::string& path, const Skeleton* skeleton = nullptr) -> bool;

        auto loadFromData(const std::vector<Mesh3DVertex>& vertices,
                          const std::vector<uint32_t>& indices) -> bool;

        auto loadFromDataSkinned(const std::vector<Mesh3DVertexSkinned>& vertices,
                                 const std::vector<uint32_t>& indices) -> bool;

        auto ensureLoaded() -> bool;

        auto getVertexBuffer() -> sg_buffer { ensureLoaded(); return m_vertexBuffer; }
        auto getIndexBuffer() -> sg_buffer { ensureLoaded(); return m_indexBuffer; }
        auto getNumIndices() -> uint32_t { ensureLoaded(); return m_numIndices; }
        auto isSkinned() const -> bool { return m_isSkinned; }

        void release();

        static auto createCube() -> std::shared_ptr<Mesh>;
        static auto createPlane() -> std::shared_ptr<Mesh>;
        static auto createSphere(int segments = 16, int rings = 16) -> std::shared_ptr<Mesh>;

    private:
        sg_buffer m_vertexBuffer = {};
        sg_buffer m_indexBuffer = {};
        uint32_t m_numIndices = 0;
        bool m_gpuReady = false;
        bool m_isSkinned = false;

        std::vector<Mesh3DVertex> m_pendingVertices;
        std::vector<Mesh3DVertexSkinned> m_pendingVerticesSkinned;
        std::vector<uint32_t> m_pendingIndices;
    };
}
