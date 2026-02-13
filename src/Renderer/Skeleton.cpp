//
//  Skeleton.cpp
//  Sapling Engine, Sprout Renderer
//

#include "Renderer/Skeleton.hpp"
#include "Core/AssetManager.hpp"

#include "ufbx/ufbx.h"

#include "Core/Logger.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace Sprout
{

static glm::mat4 ufbxMatToGlm(const ufbx_matrix& m)
{
    return glm::mat4(
        static_cast<float>(m.m00), static_cast<float>(m.m10), static_cast<float>(m.m20), 0.0f,
        static_cast<float>(m.m01), static_cast<float>(m.m11), static_cast<float>(m.m21), 0.0f,
        static_cast<float>(m.m02), static_cast<float>(m.m12), static_cast<float>(m.m22), 0.0f,
        static_cast<float>(m.m03), static_cast<float>(m.m13), static_cast<float>(m.m23), 1.0f
    );
}

auto Skeleton::loadFromFBX(const std::string& path) -> bool
{
    bones.clear();
    boneNameToIndex.clear();
    clusterToBoneIndex.clear();

    std::string full_path = AssetManager::getAssetsPath() + path;

    ufbx_load_opts opts = {};
    ufbx_error error;
    ufbx_scene* scene = ufbx_load_file(full_path.c_str(), &opts, &error);

    if (!scene) {
        Logger::error("Failed to load FBX skeleton: " + full_path + " - " + error.description.data);
        return false;
    }

    ufbx_skin_deformer* skin = nullptr;
    for (size_t i = 0; i < scene->skin_deformers.count; i++) {
        if (scene->skin_deformers.data[i]->clusters.count > 0) {
            skin = scene->skin_deformers.data[i];
            break;
        }
    }

    if (!skin) {
        Logger::error("No skin deformer found in FBX file: " + full_path);
        ufbx_free_scene(scene);
        return false;
    }

    size_t numClusters = skin->clusters.count;

    // load bones
    std::vector<Bone> unsorted(numClusters);
    std::unordered_map<std::string, int32_t> nameToCluster;

    for (size_t i = 0; i < numClusters; i++) {
        ufbx_skin_cluster* cluster = skin->clusters.data[i];
        ufbx_node* bone_node = cluster->bone_node;

        unsorted[i].name = std::string(bone_node->name.data, bone_node->name.length);
        unsorted[i].inverseBindPose = ufbxMatToGlm(cluster->geometry_to_bone);
        unsorted[i].parentIndex = -1;
        nameToCluster[unsorted[i].name] = static_cast<int32_t>(i);
    }

    // resolve parent indices
    for (size_t i = 0; i < numClusters; i++) {
        ufbx_node* bone_node = skin->clusters.data[i]->bone_node;
        ufbx_node* parent = bone_node->parent;
        while (parent) {
            std::string parent_name(parent->name.data, parent->name.length);
            auto it = nameToCluster.find(parent_name);
            if (it != nameToCluster.end()) {
                unsorted[i].parentIndex = it->second;
                break;
            }
            parent = parent->parent;
        }
    }

    // topological sort so parents always before children
    std::vector<int32_t> sortedOrder;
    sortedOrder.reserve(numClusters);
    std::vector<bool> placed(numClusters, false);

    // look for bones if parent is already placed or is -1
    while (sortedOrder.size() < numClusters) {
        bool progress = false;
        for (size_t i = 0; i < numClusters; i++) {
            if (placed[i]) continue;
            int32_t parent = unsorted[i].parentIndex;
            if (parent == -1 || placed[parent]) {
                sortedOrder.push_back(static_cast<int32_t>(i));
                placed[i] = true;
                progress = true;
            }
        }
        if (!progress) {
            for (size_t i = 0; i < numClusters; i++) {
                if (!placed[i]) {
                    sortedOrder.push_back(static_cast<int32_t>(i));
                    placed[i] = true;
                }
            }
            break;
        }
    }

    clusterToBoneIndex.resize(numClusters);
    std::vector<int32_t> clusterToSorted(numClusters);
    for (size_t sortedIdx = 0; sortedIdx < numClusters; sortedIdx++) {
        clusterToSorted[sortedOrder[sortedIdx]] = static_cast<int32_t>(sortedIdx);
    }
    clusterToBoneIndex = clusterToSorted;

    // build final sorted bones array
    bones.resize(numClusters);
    for (size_t sortedIdx = 0; sortedIdx < numClusters; sortedIdx++) {
        int32_t clusterIdx = sortedOrder[sortedIdx];
        bones[sortedIdx] = unsorted[clusterIdx];

        if (bones[sortedIdx].parentIndex >= 0) {
            bones[sortedIdx].parentIndex = clusterToSorted[bones[sortedIdx].parentIndex];
        }

        boneNameToIndex[bones[sortedIdx].name] = static_cast<int32_t>(sortedIdx);
    }

    ufbx_free_scene(scene);

    return true;
}

auto Skeleton::findBoneIndex(const std::string& name) const -> int32_t
{
    auto it = boneNameToIndex.find(name);
    if (it != boneNameToIndex.end()) {
        return it->second;
    }
    return -1;
}

} // namespace Sprout
