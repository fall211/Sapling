//
//  Animation.cpp
//  Sapling Engine, Sprout Renderer
//

#include "Renderer/Animation.hpp"
#include "Renderer/Skeleton.hpp"
#include "Core/AssetManager.hpp"

#include "ufbx/ufbx.h"

#include "Core/Logger.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Sprout
{

auto AnimationClip::loadFromFBX(const std::string& path, const Skeleton& skeleton,
                                const std::string& clipName) -> bool
{
    tracks.clear();
    duration = 0.0f;
    name = clipName;

    std::string full_path = AssetManager::getAssetsPath() + path;

    ufbx_load_opts opts = {};
    ufbx_error error;
    ufbx_scene* scene = ufbx_load_file(full_path.c_str(), &opts, &error);

    if (!scene) {
        Logger::error("Failed to load FBX animation: " + full_path + " - " + error.description.data);
        return false;
    }

    // find animation stack
    ufbx_anim_stack* anim_stack = nullptr;
    if (clipName.empty()) {
        if (scene->anim_stacks.count > 0) {
            anim_stack = scene->anim_stacks.data[0];
        }
    } else {
        for (size_t i = 0; i < scene->anim_stacks.count; i++) {
            ufbx_anim_stack* stack = scene->anim_stacks.data[i];
            std::string stack_name(stack->name.data, stack->name.length);
            if (stack_name == clipName) {
                anim_stack = stack;
                break;
            }
        }
    }

    if (!anim_stack) {
        Logger::error("No animation found in FBX file: " + full_path);
        ufbx_free_scene(scene);
        return false;
    }

    // find the skin deformer with actual clusters (must match Skeleton ordering)
    ufbx_skin_deformer* skin = nullptr;
    for (size_t i = 0; i < scene->skin_deformers.count; i++) {
        if (scene->skin_deformers.data[i]->clusters.count > 0) {
            skin = scene->skin_deformers.data[i];
            break;
        }
    }

    if (!skin) {
        Logger::error("No skin deformer found for animation: " + full_path);
        ufbx_free_scene(scene);
        return false;
    }

    name = std::string(anim_stack->name.data, anim_stack->name.length);

    double time_begin = anim_stack->time_begin;
    double time_end = anim_stack->time_end;
    duration = static_cast<float>(time_end - time_begin);

    const float sampleRate = 30.0f;
    const int numSamples = static_cast<int>(duration * sampleRate) + 1;

    std::vector<BoneTrack> clusterTracks(skin->clusters.count);
    for (size_t i = 0; i < skin->clusters.count; i++) {
        ufbx_node* bone_node = skin->clusters.data[i]->bone_node;
        BoneTrack& track = clusterTracks[i];

        for (int sample = 0; sample < numSamples; sample++) {
            float t = static_cast<float>(sample) / sampleRate;
            double ufbx_time = time_begin + t;

            ufbx_transform transform = ufbx_evaluate_transform(anim_stack->anim, bone_node, ufbx_time);

            KeyframeVec3 posKey;
            posKey.time = t;
            posKey.value = glm::vec3(
                static_cast<float>(transform.translation.x),
                static_cast<float>(transform.translation.y),
                static_cast<float>(transform.translation.z)
            );
            track.positions.push_back(posKey);

            KeyframeQuat rotKey;
            rotKey.time = t;
            rotKey.value = glm::quat(
                static_cast<float>(transform.rotation.w),
                static_cast<float>(transform.rotation.x),
                static_cast<float>(transform.rotation.y),
                static_cast<float>(transform.rotation.z)
            );
            track.rotations.push_back(rotKey);

            KeyframeVec3 scaleKey;
            scaleKey.time = t;
            scaleKey.value = glm::vec3(
                static_cast<float>(transform.scale.x),
                static_cast<float>(transform.scale.y),
                static_cast<float>(transform.scale.z)
            );
            track.scales.push_back(scaleKey);
        }
    }

    tracks.resize(skin->clusters.count);
    if (!skeleton.clusterToBoneIndex.empty()) {
        if (skin->clusters.count > skeleton.clusterToBoneIndex.size()) {
            Logger::error("AnimationClip: skeleton/animation mismatch — animation has " +
                std::to_string(skin->clusters.count) + " clusters but skeleton has " +
                std::to_string(skeleton.clusterToBoneIndex.size()));
            ufbx_free_scene(scene);
            return false;
        }
        for (size_t clusterIdx = 0; clusterIdx < skin->clusters.count; clusterIdx++) {
            int32_t sortedIdx = skeleton.clusterToBoneIndex[clusterIdx];
            if (sortedIdx < 0 || static_cast<size_t>(sortedIdx) >= tracks.size()) continue;
            tracks[sortedIdx] = std::move(clusterTracks[clusterIdx]);
        }
    } else {
        tracks = std::move(clusterTracks);
    }

    ufbx_free_scene(scene);
    return true;
}

void AnimationClip::sample(float time, std::vector<glm::mat4>& outBoneMatrices,
                           const Skeleton& skeleton) const
{
    if (tracks.empty() || skeleton.bones.empty()) {
        return;
    }

    size_t numBones = skeleton.bones.size();
    outBoneMatrices.resize(numBones, glm::mat4(1.0f));

    std::vector<glm::mat4> worldTransforms(numBones, glm::mat4(1.0f));

    for (size_t i = 0; i < std::min(tracks.size(), numBones); i++) {
        const BoneTrack& track = tracks[i];
        const Bone& bone = skeleton.bones[i];

        glm::vec3 position = interpolateVec3(track.positions, time);
        glm::quat rotation = interpolateQuat(track.rotations, time);
        glm::vec3 scale = interpolateVec3(track.scales, time);

        glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), position)
                                 * glm::toMat4(rotation)
                                 * glm::scale(glm::mat4(1.0f), scale);

        if (bone.parentIndex >= 0 && bone.parentIndex < static_cast<int32_t>(numBones)) {
            worldTransforms[i] = worldTransforms[bone.parentIndex] * localTransform;
        } else {
            worldTransforms[i] = localTransform;
        }

        outBoneMatrices[i] = worldTransforms[i] * bone.inverseBindPose;
    }
}

auto AnimationClip::interpolateVec3(const std::vector<KeyframeVec3>& keyframes, float time) const -> glm::vec3
{
    if (keyframes.empty()) return glm::vec3(0.0f);
    if (keyframes.size() == 1) return keyframes[0].value;
    if (time <= keyframes.front().time) return keyframes.front().value;
    if (time >= keyframes.back().time) return keyframes.back().value;

    // binary search for the interval containing 'time'
    auto it = std::lower_bound(keyframes.begin(), keyframes.end(), time,
        [](const KeyframeVec3& kf, float t) { return kf.time < t; });

    size_t i = static_cast<size_t>(it - keyframes.begin());
    if (i == 0) return keyframes[0].value;

    float t = (time - keyframes[i - 1].time) / (keyframes[i].time - keyframes[i - 1].time);
    return glm::mix(keyframes[i - 1].value, keyframes[i].value, t);
}

auto AnimationClip::interpolateQuat(const std::vector<KeyframeQuat>& keyframes, float time) const -> glm::quat
{
    if (keyframes.empty()) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    if (keyframes.size() == 1) return keyframes[0].value;
    if (time <= keyframes.front().time) return keyframes.front().value;
    if (time >= keyframes.back().time) return keyframes.back().value;

    // binary search for the interval containing 'time'
    auto it = std::lower_bound(keyframes.begin(), keyframes.end(), time,
        [](const KeyframeQuat& kf, float t) { return kf.time < t; });

    size_t i = static_cast<size_t>(it - keyframes.begin());
    if (i == 0) return keyframes[0].value;

    float t = (time - keyframes[i - 1].time) / (keyframes[i].time - keyframes[i - 1].time);
    return glm::slerp(keyframes[i - 1].value, keyframes[i].value, t);
}

} // namespace Sprout
