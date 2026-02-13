//
//  Forward3DPass.cpp
//  Sapling Engine, Sprout Renderer
//

#include "Renderer/Forward3DPass.hpp"
#include "Renderer/Sprout.hpp"
#include "Renderer/mesh3d.h"
#include "Renderer/mesh3d_skinned.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstring>

namespace Sprout
{

void Forward3DPass::ensureResources()
{
    if (m_resourcesCreated) return;

    auto make1x1 = [](uint32_t pixel, const char* label) -> sg_image {
        sg_image_desc desc = {};
        desc.width = 1;
        desc.height = 1;
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        desc.data.subimage[0][0].ptr = &pixel;
        desc.data.subimage[0][0].size = 4;
        desc.label = label;
        return sg_make_image(&desc);
    };

    m_whiteTex = make1x1(0xFFFFFFFF, "forward3d-white-tex");
    m_flatNormalTex = make1x1(0xFFFF8080, "forward3d-flat-normal"); // RGBA(128,128,255,255) = flat normal pointing up
    m_blackTex = make1x1(0xFF000000, "forward3d-black-tex");
    m_midGrayTex = make1x1(0xFF808080, "forward3d-midgray-tex");

    // create sampler with linear filtering for 3D
    sg_sampler_desc smp_desc = {};
    smp_desc.min_filter = SG_FILTER_LINEAR;
    smp_desc.mag_filter = SG_FILTER_LINEAR;
    smp_desc.mipmap_filter = SG_FILTER_LINEAR;
    smp_desc.wrap_u = SG_WRAP_REPEAT;
    smp_desc.wrap_v = SG_WRAP_REPEAT;
    smp_desc.label = "forward3d-sampler";
    m_sampler = sg_make_sampler(&smp_desc);

    m_resourcesCreated = true;
}

void Forward3DPass::submit(const std::shared_ptr<Mesh>& mesh,
                           const std::shared_ptr<Material>& material,
                           const glm::mat4& modelMatrix,
                           bool isSkinned,
                           const std::vector<glm::mat4>* boneMatrices)
{
    m_drawRequests.push_back({mesh, material, modelMatrix, isSkinned, boneMatrices});
}

void Forward3DPass::clear()
{
    m_drawRequests.clear();
}

void Forward3DPass::execute(Window& window)
{
    if (!enabled || m_drawRequests.empty()) return;

    ensureResources();

    // sort by material pipeline to minimize state changes
    std::sort(m_drawRequests.begin(), m_drawRequests.end(),
        [](const MeshDrawRequest& a, const MeshDrawRequest& b) {
            return a.material->getPipeline().id < b.material->getPipeline().id;
        });

    glm::mat4 viewProj = sceneData.projectionMatrix * sceneData.viewMatrix;

    uint32_t currentPipelineId = SG_INVALID_ID;

    for (const auto& request : m_drawRequests) {
        if (!request.mesh || !request.material) continue;

        // switch pipeline if needed
        if (request.material->getPipeline().id != currentPipelineId) {
            sg_apply_pipeline(request.material->getPipeline());
            currentPipelineId = request.material->getPipeline().id;
        }

        // bindings
        sg_bindings bindings = {};
        bindings.vertex_buffers[0] = request.mesh->getVertexBuffer();
        bindings.index_buffer = request.mesh->getIndexBuffer();

        // texture helper: use material texture if available, otherwise default
        auto bindTex = [&](int slot, const std::shared_ptr<Texture>& tex, sg_image fallback) {
            if (tex) {
                bindings.images[slot] = tex->getImageHandle();
            } else {
                bindings.images[slot] = fallback;
            }
        };

        bindTex(IMG_diffuse_tex, request.material->diffuseTexture, m_whiteTex);
        bindTex(IMG_normal_tex, request.material->normalTexture, m_flatNormalTex);
        bindTex(IMG_emissive_tex, request.material->emissiveTexture, m_blackTex);
        bindTex(IMG_roughness_tex, request.material->roughnessTexture, m_midGrayTex);

        // sampler — use diffuse texture's sampler if available, otherwise default
        if (request.material->diffuseTexture) {
            sg_sampler texSampler = request.material->diffuseTexture->getSampler();
            bindings.samplers[SMP_default_sampler] = (texSampler.id != SG_INVALID_ID) ? texSampler : m_sampler;
        } else {
            bindings.samplers[SMP_default_sampler] = m_sampler;
        }

        sg_apply_bindings(&bindings);

        // compute CPU-side normal matrix
        glm::mat4 mvp = viewProj * request.modelMatrix;
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(request.modelMatrix)));

        // vertex shader uniforms
        if (request.isSkinned && request.boneMatrices) {
            mesh3d_skinned_vs_params_t vs_params = {};
            std::memcpy(vs_params.mvp, glm::value_ptr(mvp), sizeof(float) * 16);
            std::memcpy(vs_params.model, glm::value_ptr(request.modelMatrix), sizeof(float) * 16);
            std::memcpy(vs_params.normal_mat_c0, glm::value_ptr(normalMatrix[0]), sizeof(float) * 3);
            std::memcpy(vs_params.normal_mat_c1, glm::value_ptr(normalMatrix[1]), sizeof(float) * 3);
            std::memcpy(vs_params.normal_mat_c2, glm::value_ptr(normalMatrix[2]), sizeof(float) * 3);

            size_t numBones = std::min(request.boneMatrices->size(), size_t(64));
            for (size_t i = 0; i < numBones; i++) {
                std::memcpy(vs_params.bone_matrices[i], glm::value_ptr((*request.boneMatrices)[i]), sizeof(float) * 16);
            }

            sg_apply_uniforms(UB_mesh3d_skinned_vs_params, SG_RANGE(vs_params));
        } else {
            mesh3d_vs_params_t vs_params = {};
            std::memcpy(vs_params.mvp, glm::value_ptr(mvp), sizeof(float) * 16);
            std::memcpy(vs_params.model, glm::value_ptr(request.modelMatrix), sizeof(float) * 16);
            std::memcpy(vs_params.normal_mat_c0, glm::value_ptr(normalMatrix[0]), sizeof(float) * 3);
            std::memcpy(vs_params.normal_mat_c1, glm::value_ptr(normalMatrix[1]), sizeof(float) * 3);
            std::memcpy(vs_params.normal_mat_c2, glm::value_ptr(normalMatrix[2]), sizeof(float) * 3);
            sg_apply_uniforms(UB_mesh3d_vs_params, SG_RANGE(vs_params));
        }

        // fragment shader uniforms — shared packing helper lambda
        auto packFsParams = [&](auto& fs_params) {
            fs_params.camera_pos_and_ambient[0] = sceneData.cameraPosition.x;
            fs_params.camera_pos_and_ambient[1] = sceneData.cameraPosition.y;
            fs_params.camera_pos_and_ambient[2] = sceneData.cameraPosition.z;
            fs_params.camera_pos_and_ambient[3] = sceneData.ambientStrength;

            fs_params.base_color[0] = request.material->properties.baseColor.r;
            fs_params.base_color[1] = request.material->properties.baseColor.g;
            fs_params.base_color[2] = request.material->properties.baseColor.b;
            fs_params.base_color[3] = request.material->properties.baseColor.a;

            fs_params.material_props[0] = request.material->properties.specularStrength;
            fs_params.material_props[1] = request.material->properties.shininess;
            fs_params.material_props[2] = request.material->properties.emissiveStrength;
            fs_params.material_props[3] = 0.0f;

            for (int i = 0; i < sceneData.numLights && i < MAX_LIGHTS; i++) {
                const auto& light = sceneData.lights[i];
                fs_params.light_pos_type[i][0] = light.positionOrDirection.x;
                fs_params.light_pos_type[i][1] = light.positionOrDirection.y;
                fs_params.light_pos_type[i][2] = light.positionOrDirection.z;
                fs_params.light_pos_type[i][3] = light.type;

                fs_params.light_color_range[i][0] = light.color.x;
                fs_params.light_color_range[i][1] = light.color.y;
                fs_params.light_color_range[i][2] = light.color.z;
                fs_params.light_color_range[i][3] = light.range;
            }

            fs_params.light_count[0] = static_cast<float>(sceneData.numLights);
        };

        if (request.isSkinned) {
            mesh3d_skinned_fs_params_t fs_params = {};
            packFsParams(fs_params);
            sg_apply_uniforms(UB_mesh3d_skinned_fs_params, SG_RANGE(fs_params));
        } else {
            mesh3d_fs_params_t fs_params = {};
            packFsParams(fs_params);
            sg_apply_uniforms(UB_mesh3d_fs_params, SG_RANGE(fs_params));
        }

        sg_draw(0, request.mesh->getNumIndices(), 1);
    }
}

} // namespace Sprout
