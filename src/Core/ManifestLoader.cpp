//
//  ManifestLoader.cpp
//  SaplingEngine
//

#include "Core/ManifestLoader.hpp"
#include "Core/AssetManager.hpp"
#include "Core/Logger.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

auto ManifestLoader::getManifestFullPath(const std::string& manifestPath) -> std::string
{
    return (std::filesystem::path(AssetManager::getAssetsPath()) / manifestPath).string();
}

auto ManifestLoader::loadJson(const std::string& manifestPath) -> nlohmann::json
{
    const std::string fullPath = getManifestFullPath(manifestPath);
    std::ifstream file(fullPath);

    if (!file.is_open())
    {
        Logger::error("ManifestLoader: failed to open manifest file: " + fullPath);
        return nlohmann::json::object();
    }

    try
    {
        nlohmann::json manifestJson;
        file >> manifestJson;
        return manifestJson;
    }
    catch (const nlohmann::json::exception& e)
    {
        Logger::error("ManifestLoader: JSON parse error in " + fullPath + ": " + e.what());
        return nlohmann::json::object();
    }
}
