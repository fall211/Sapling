//
//  Mesh.cpp
//  Sapling Engine, Sprout Renderer
//

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#include "Renderer/Mesh.hpp"
#include "Renderer/Skeleton.hpp"

#include "ufbx/ufbx.h"

#include <cmath>
#include "Core/AssetManager.hpp"
#include "Core/Logger.hpp"
#include <unordered_map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Sprout
{

void computeTangents(std::vector<Mesh3DVertex>& vertices, const std::vector<uint32_t>& indices)
{
    std::vector<glm::vec3> tan1(vertices.size(), glm::vec3(0.0f));
    std::vector<glm::vec3> tan2(vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        uint32_t i0 = indices[i], i1 = indices[i+1], i2 = indices[i+2];
        const auto& v0 = vertices[i0];
        const auto& v1 = vertices[i1];
        const auto& v2 = vertices[i2];

        glm::vec3 e1 = v1.position - v0.position;
        glm::vec3 e2 = v2.position - v0.position;
        glm::vec2 duv1 = v1.texcoord - v0.texcoord;
        glm::vec2 duv2 = v2.texcoord - v0.texcoord;

        float r = duv1.x * duv2.y - duv2.x * duv1.y;
        if (std::abs(r) < 1e-8f) continue;
        r = 1.0f / r;

        glm::vec3 sdir((duv2.y * e1.x - duv1.y * e2.x) * r,
                        (duv2.y * e1.y - duv1.y * e2.y) * r,
                        (duv2.y * e1.z - duv1.y * e2.z) * r);
        glm::vec3 tdir((duv1.x * e2.x - duv2.x * e1.x) * r,
                        (duv1.x * e2.y - duv2.x * e1.y) * r,
                        (duv1.x * e2.z - duv2.x * e1.z) * r);

        tan1[i0] += sdir; tan1[i1] += sdir; tan1[i2] += sdir;
        tan2[i0] += tdir; tan2[i1] += tdir; tan2[i2] += tdir;
    }

    for (size_t i = 0; i < vertices.size(); i++) {
        const glm::vec3& n = vertices[i].normal;
        const glm::vec3& t = tan1[i];
        glm::vec3 tangent = t - n * glm::dot(n, t);
        float len = glm::length(tangent);
        if (len > 1e-6f) tangent /= len;
        else tangent = glm::vec3(1, 0, 0);
        float w = (glm::dot(glm::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f;
        vertices[i].tangent = glm::vec4(tangent, w);
    }
}

void computeTangentsSkinned(std::vector<Mesh3DVertexSkinned>& vertices, const std::vector<uint32_t>& indices)
{
    std::vector<glm::vec3> tan1(vertices.size(), glm::vec3(0.0f));
    std::vector<glm::vec3> tan2(vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        uint32_t i0 = indices[i], i1 = indices[i+1], i2 = indices[i+2];
        const auto& v0 = vertices[i0];
        const auto& v1 = vertices[i1];
        const auto& v2 = vertices[i2];

        glm::vec3 e1 = v1.position - v0.position;
        glm::vec3 e2 = v2.position - v0.position;
        glm::vec2 duv1 = v1.texcoord - v0.texcoord;
        glm::vec2 duv2 = v2.texcoord - v0.texcoord;

        float r = duv1.x * duv2.y - duv2.x * duv1.y;
        if (std::abs(r) < 1e-8f) continue;
        r = 1.0f / r;

        glm::vec3 sdir((duv2.y * e1.x - duv1.y * e2.x) * r,
                        (duv2.y * e1.y - duv1.y * e2.y) * r,
                        (duv2.y * e1.z - duv1.y * e2.z) * r);
        glm::vec3 tdir((duv1.x * e2.x - duv2.x * e1.x) * r,
                        (duv1.x * e2.y - duv2.x * e1.y) * r,
                        (duv1.x * e2.z - duv2.x * e1.z) * r);

        tan1[i0] += sdir; tan1[i1] += sdir; tan1[i2] += sdir;
        tan2[i0] += tdir; tan2[i1] += tdir; tan2[i2] += tdir;
    }

    for (size_t i = 0; i < vertices.size(); i++) {
        const glm::vec3& n = vertices[i].normal;
        const glm::vec3& t = tan1[i];
        glm::vec3 tangent = t - n * glm::dot(n, t);
        float len = glm::length(tangent);
        if (len > 1e-6f) tangent /= len;
        else tangent = glm::vec3(1, 0, 0);
        float w = (glm::dot(glm::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f;
        vertices[i].tangent = glm::vec4(tangent, w);
    }
}

Mesh::~Mesh()
{
    release();
}

void Mesh::release()
{
    if (m_vertexBuffer.id != SG_INVALID_ID) {
        sg_destroy_buffer(m_vertexBuffer);
        m_vertexBuffer.id = SG_INVALID_ID;
    }
    if (m_indexBuffer.id != SG_INVALID_ID) {
        sg_destroy_buffer(m_indexBuffer);
        m_indexBuffer.id = SG_INVALID_ID;
    }
    m_numIndices = 0;
    m_gpuReady = false;
    m_isSkinned = false;
    m_pendingVertices.clear();
    m_pendingVerticesSkinned.clear();
    m_pendingIndices.clear();
}

auto Mesh::loadFromData(const std::vector<Mesh3DVertex>& vertices,
                        const std::vector<uint32_t>& indices) -> bool
{
    release();

    // cpu data only. gpu upload deferred until ensureLoaded()
    m_pendingVertices = vertices;
    m_pendingIndices = indices;
    computeTangents(m_pendingVertices, m_pendingIndices);
    m_numIndices = static_cast<uint32_t>(indices.size());
    m_isSkinned = false;
    m_gpuReady = false;
    return true;
}

auto Mesh::loadFromDataSkinned(const std::vector<Mesh3DVertexSkinned>& vertices,
                               const std::vector<uint32_t>& indices) -> bool
{
    release();

    m_pendingVerticesSkinned = vertices;
    m_pendingIndices = indices;
    computeTangentsSkinned(m_pendingVerticesSkinned, m_pendingIndices);
    m_numIndices = static_cast<uint32_t>(indices.size());
    m_isSkinned = true;
    m_gpuReady = false;
    return true;
}

auto Mesh::ensureLoaded() -> bool
{
    if (m_gpuReady) return true;

    if (m_isSkinned) {
        if (m_pendingVerticesSkinned.empty()) return false;

        sg_buffer_desc vbuf_desc = {};
        vbuf_desc.data.ptr = m_pendingVerticesSkinned.data();
        vbuf_desc.data.size = m_pendingVerticesSkinned.size() * sizeof(Mesh3DVertexSkinned);
        vbuf_desc.label = "mesh-vertices-skinned";
        m_vertexBuffer = sg_make_buffer(&vbuf_desc);

        m_pendingVerticesSkinned.clear();
        m_pendingVerticesSkinned.shrink_to_fit();
    } else {
        if (m_pendingVertices.empty()) return false;

        sg_buffer_desc vbuf_desc = {};
        vbuf_desc.data.ptr = m_pendingVertices.data();
        vbuf_desc.data.size = m_pendingVertices.size() * sizeof(Mesh3DVertex);
        vbuf_desc.label = "mesh-vertices";
        m_vertexBuffer = sg_make_buffer(&vbuf_desc);

        m_pendingVertices.clear();
        m_pendingVertices.shrink_to_fit();
    }

    sg_buffer_desc ibuf_desc = {};
    ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuf_desc.data.ptr = m_pendingIndices.data();
    ibuf_desc.data.size = m_pendingIndices.size() * sizeof(uint32_t);
    ibuf_desc.label = "mesh-indices";
    m_indexBuffer = sg_make_buffer(&ibuf_desc);

    m_numIndices = static_cast<uint32_t>(m_pendingIndices.size());

    m_pendingIndices.clear();
    m_pendingIndices.shrink_to_fit();

    m_gpuReady = true;
    return true;
}

auto Mesh::loadOBJ(const std::string& path) -> bool
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string full_path = AssetManager::getAssetsPath() + path;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, full_path.c_str())) {
        Logger::error("Failed to load OBJ: " + full_path + " - " + warn + err);
        return false;
    }

    std::vector<Mesh3DVertex> vertices;
    std::vector<uint32_t> indices;

    struct IndexHash {
        size_t operator()(const tinyobj::index_t& idx) const {
            size_t h = std::hash<int>()(idx.vertex_index);
            h ^= std::hash<int>()(idx.normal_index) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>()(idx.texcoord_index) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
    struct IndexEqual {
        bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const {
            return a.vertex_index == b.vertex_index &&
                   a.normal_index == b.normal_index &&
                   a.texcoord_index == b.texcoord_index;
        }
    };
    std::unordered_map<tinyobj::index_t, uint32_t, IndexHash, IndexEqual> uniqueVertices;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            auto it = uniqueVertices.find(index);
            if (it != uniqueVertices.end()) {
                indices.push_back(it->second);
                continue;
            }

            Mesh3DVertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.texcoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            auto idx = static_cast<uint32_t>(vertices.size());
            uniqueVertices[index] = idx;
            vertices.push_back(vertex);
            indices.push_back(idx);
        }
    }

    return loadFromData(vertices, indices);
}

auto Mesh::createCube() -> std::shared_ptr<Mesh>
{
    std::vector<Mesh3DVertex> vertices = {
        {{-0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {0, 1}},
        {{ 0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {1, 1}},
        {{ 0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {1, 0}},
        {{-0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, { 0, 0,-1}, {0, 1}},
        {{-0.5f, -0.5f, -0.5f}, { 0, 0,-1}, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, { 0, 0,-1}, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, { 0, 0,-1}, {0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, { 1, 0, 0}, {0, 1}},
        {{ 0.5f, -0.5f, -0.5f}, { 1, 0, 0}, {1, 1}},
        {{ 0.5f,  0.5f, -0.5f}, { 1, 0, 0}, {1, 0}},
        {{ 0.5f,  0.5f,  0.5f}, { 1, 0, 0}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 1}},
        {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1, 1}},
        {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1, 0}},
        {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0, 0}},
        {{-0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {0, 1}},
        {{ 0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {1, 1}},
        {{ 0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {1, 0}},
        {{-0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {0, 1}},
        {{ 0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {1, 1}},
        {{ 0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {1, 0}},
        {{-0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {0, 0}},
    };

    std::vector<uint32_t> indices;
    for (uint32_t face = 0; face < 6; face++) {
        uint32_t base = face * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    auto mesh = std::make_shared<Mesh>();
    mesh->loadFromData(vertices, indices);
    return mesh;
}

auto Mesh::createPlane() -> std::shared_ptr<Mesh>
{
    std::vector<Mesh3DVertex> vertices = {
        {{-0.5f, 0.0f,  0.5f}, {0, 1, 0}, {0, 0}},
        {{ 0.5f, 0.0f,  0.5f}, {0, 1, 0}, {1, 0}},
        {{ 0.5f, 0.0f, -0.5f}, {0, 1, 0}, {1, 1}},
        {{-0.5f, 0.0f, -0.5f}, {0, 1, 0}, {0, 1}},
    };

    std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

    auto mesh = std::make_shared<Mesh>();
    mesh->loadFromData(vertices, indices);
    return mesh;
}

auto Mesh::createSphere(int segments, int rings) -> std::shared_ptr<Mesh>
{
    std::vector<Mesh3DVertex> vertices;
    std::vector<uint32_t> indices;

    for (int y = 0; y <= rings; y++) {
        for (int x = 0; x <= segments; x++) {
            float xSegment = static_cast<float>(x) / static_cast<float>(segments);
            float ySegment = static_cast<float>(y) / static_cast<float>(rings);
            float xPos = std::cos(xSegment * 2.0f * static_cast<float>(M_PI)) * std::sin(ySegment * static_cast<float>(M_PI));
            float yPos = std::cos(ySegment * static_cast<float>(M_PI));
            float zPos = std::sin(xSegment * 2.0f * static_cast<float>(M_PI)) * std::sin(ySegment * static_cast<float>(M_PI));

            Mesh3DVertex v{};
            v.position = {xPos * 0.5f, yPos * 0.5f, zPos * 0.5f};
            v.normal = glm::normalize(glm::vec3(xPos, yPos, zPos));
            v.texcoord = {xSegment, ySegment};
            vertices.push_back(v);
        }
    }

    for (int y = 0; y < rings; y++) {
        for (int x = 0; x < segments; x++) {
            auto i0 = static_cast<uint32_t>(y * (segments + 1) + x);
            auto i1 = static_cast<uint32_t>(i0 + 1);
            auto i2 = static_cast<uint32_t>((y + 1) * (segments + 1) + x);
            auto i3 = static_cast<uint32_t>(i2 + 1);
            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    auto mesh = std::make_shared<Mesh>();
    mesh->loadFromData(vertices, indices);
    return mesh;
}

auto Mesh::loadFBX(const std::string& path, const Skeleton* skeleton) -> bool
{
    std::string full_path = AssetManager::getAssetsPath() + path;

    ufbx_load_opts opts = {};
    ufbx_error error;
    ufbx_scene* scene = ufbx_load_file(full_path.c_str(), &opts, &error);

    if (!scene) {
        Logger::error("Failed to load FBX mesh: " + full_path + " - " + error.description.data);
        return false;
    }

    // find first mesh
    ufbx_mesh* fbx_mesh = nullptr;
    for (size_t i = 0; i < scene->meshes.count; i++) {
        fbx_mesh = scene->meshes.data[i];
        if (fbx_mesh->num_vertices > 0) {
            break;
        }
    }

    if (!fbx_mesh) {
        Logger::error("No mesh found in FBX file: " + full_path);
        ufbx_free_scene(scene);
        return false;
    }

    // check if skinned
    bool hasSkin = fbx_mesh->skin_deformers.count > 0;

    if (hasSkin) {
        std::vector<Mesh3DVertexSkinned> vertices;
        std::vector<uint32_t> indices;

        for (size_t face_ix = 0; face_ix < fbx_mesh->num_faces; face_ix++) {
            ufbx_face face = fbx_mesh->faces.data[face_ix];

            // triangulate
            for (size_t tri = 0; tri < face.num_indices - 2; tri++) {
                size_t tri_indices[3] = { 0, tri + 1, tri + 2 };

                for (size_t i = 0; i < 3; i++) {
                    size_t corner = face.index_begin + tri_indices[i];

                    uint32_t vertex_ix = fbx_mesh->vertex_indices.data[corner];
                    uint32_t uv_ix = fbx_mesh->vertex_uv.exists ? fbx_mesh->vertex_uv.indices.data[corner] : 0;
                    uint32_t normal_ix = fbx_mesh->vertex_normal.indices.data[corner];

                    Mesh3DVertexSkinned vertex{};

                    ufbx_vec3 pos = fbx_mesh->vertex_position.values.data[vertex_ix];
                    vertex.position = glm::vec3(
                        static_cast<float>(pos.x),
                        static_cast<float>(pos.y),
                        static_cast<float>(pos.z)
                    );

                    ufbx_vec3 normal = fbx_mesh->vertex_normal.values.data[normal_ix];
                    vertex.normal = glm::vec3(
                        static_cast<float>(normal.x),
                        static_cast<float>(normal.y),
                        static_cast<float>(normal.z)
                    );

                    if (fbx_mesh->vertex_uv.exists) {
                        ufbx_vec2 uv = fbx_mesh->vertex_uv.values.data[uv_ix];
                        vertex.texcoord = glm::vec2(
                            static_cast<float>(uv.x),
                            1.0f - static_cast<float>(uv.y)  // Flip V coordinate
                        );
                    }

                    vertex.boneIndices = glm::vec4(0.0f);
                    vertex.boneWeights = glm::vec4(0.0f);

                    if (fbx_mesh->skin_deformers.count > 0) {
                        ufbx_skin_deformer* skin = fbx_mesh->skin_deformers.data[0];
                        ufbx_skin_vertex skin_vertex = skin->vertices.data[vertex_ix];

                        size_t num_weights = skin_vertex.num_weights < 4 ? skin_vertex.num_weights : 4;
                        for (size_t w = 0; w < num_weights; w++) {
                            ufbx_skin_weight weight = skin->weights.data[skin_vertex.weight_begin + w];
                            int32_t boneIdx = static_cast<int32_t>(weight.cluster_index);
                            if (skeleton && !skeleton->clusterToBoneIndex.empty()) {
                                boneIdx = skeleton->clusterToBoneIndex[boneIdx];
                            }
                            vertex.boneIndices[w] = static_cast<float>(boneIdx);
                            vertex.boneWeights[w] = static_cast<float>(weight.weight);
                        }

                        float weight_sum = vertex.boneWeights.x + vertex.boneWeights.y +
                                         vertex.boneWeights.z + vertex.boneWeights.w;
                        if (weight_sum > 0.0f) {
                            vertex.boneWeights /= weight_sum;
                        }
                    }

                    vertices.push_back(vertex);
                    indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
                }
            }
        }

        ufbx_free_scene(scene);
        return loadFromDataSkinned(vertices, indices);

    } else {
        // no skin
        std::vector<Mesh3DVertex> vertices;
        std::vector<uint32_t> indices;

        size_t num_triangles = fbx_mesh->num_triangles;
        vertices.reserve(num_triangles * 3);
        indices.reserve(num_triangles * 3);

        for (size_t face_ix = 0; face_ix < fbx_mesh->num_faces; face_ix++) {
            ufbx_face face = fbx_mesh->faces.data[face_ix];

            // fan-triangulate n-gons (matches skinned path)
            for (size_t tri = 0; tri < face.num_indices - 2; tri++) {
                size_t tri_indices[3] = { 0, tri + 1, tri + 2 };

                for (size_t i = 0; i < 3; i++) {
                    size_t corner = face.index_begin + tri_indices[i];

                    uint32_t vertex_ix = fbx_mesh->vertex_indices.data[corner];
                    uint32_t normal_ix = fbx_mesh->vertex_normal.indices.data[corner];

                    Mesh3DVertex vertex{};

                    ufbx_vec3 pos = fbx_mesh->vertex_position.values.data[vertex_ix];
                    vertex.position = glm::vec3(
                        static_cast<float>(pos.x),
                        static_cast<float>(pos.y),
                        static_cast<float>(pos.z)
                    );

                    ufbx_vec3 normal = fbx_mesh->vertex_normal.values.data[normal_ix];
                    vertex.normal = glm::vec3(
                        static_cast<float>(normal.x),
                        static_cast<float>(normal.y),
                        static_cast<float>(normal.z)
                    );

                    if (fbx_mesh->vertex_uv.exists) {
                        uint32_t uv_ix = fbx_mesh->vertex_uv.indices.data[corner];
                        ufbx_vec2 uv = fbx_mesh->vertex_uv.values.data[uv_ix];
                        vertex.texcoord = glm::vec2(
                            static_cast<float>(uv.x),
                            1.0f - static_cast<float>(uv.y)
                        );
                    }

                    vertices.push_back(vertex);
                    indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
                }
            }
        }

        ufbx_free_scene(scene);
        return loadFromData(vertices, indices);
    }
}

} // namespace Sprout
