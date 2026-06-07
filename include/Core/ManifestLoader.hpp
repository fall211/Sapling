//
//  ManifestLoader.hpp
//  SaplingEngine
//

#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

class ManifestLoader
{
public:
    static auto loadJson(const std::string& manifestPath) -> nlohmann::json;
    static auto getManifestFullPath(const std::string& manifestPath) -> std::string;
};
