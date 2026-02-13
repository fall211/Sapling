//
//  Animation.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <vector>
#include <memory>

namespace Sprout
{
    class Skeleton;

    struct KeyframeVec3
    {
        float time;
        glm::vec3 value;
    };

    struct KeyframeQuat
    {
        float time;
        glm::quat value;
    };

    struct BoneTrack
    {
        std::vector<KeyframeVec3> positions;
        std::vector<KeyframeQuat> rotations;
        std::vector<KeyframeVec3> scales;
    };

    class AnimationClip
    {
    public:
        AnimationClip() = default;
        ~AnimationClip() = default;

        auto loadFromFBX(const std::string& path, const Skeleton& skeleton,
                         const std::string& clipName = "") -> bool;

        void sample(float time, std::vector<glm::mat4>& outBoneMatrices,
                   const Skeleton& skeleton) const;

        std::string name;
        float duration = 0.0f;  // seconds
        std::vector<BoneTrack> tracks;

    private:
        auto interpolateVec3(const std::vector<KeyframeVec3>& keyframes, float time) const -> glm::vec3;
        auto interpolateQuat(const std::vector<KeyframeQuat>& keyframes, float time) const -> glm::quat;
    };
}
