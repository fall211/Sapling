//
//  Skeleton.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace Sprout
{
    struct Bone
    {
        std::string name;
        int32_t parentIndex;
        glm::mat4 inverseBindPose;
    };

    class Skeleton
    {
    public:
        Skeleton() = default;
        ~Skeleton() = default;

        auto loadFromFBX(const std::string& path) -> bool;
        auto findBoneIndex(const std::string& name) const -> int32_t;

        std::vector<Bone> bones;
        std::unordered_map<std::string, int32_t> boneNameToIndex;
        std::vector<int32_t> clusterToBoneIndex;
    };
}
