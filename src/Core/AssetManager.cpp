//
//  AssetManager.cpp
//  SaplingEngine, Seedbank Asset Manager
//

#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Core/ManifestLoader.hpp"
#include "Renderer/Font.hpp"
#include "Utility/Debug.hpp"

#include "fmod_common.h"
#include <nlohmann/json.hpp>

#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>

AssetManager* AssetManager::Instance = nullptr;
std::string AssetManager::s_runtimeAssetsPath = "";
float AssetManager::s_pixelsPerUnit = 16.0f;

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif


void AssetManager::initialize()
{
    if (!Instance)
    {
        Instance = new AssetManager();
    }

    Logger::info("AudioEngine init completed");
}

void AssetManager::cleanUp()
{
    for (auto& pair : Instance->m_textures) {
        pair.second->release();
    }
    Instance->m_textures.clear();

    for (auto& pair : Instance->m_imageTextures) {
        pair.second->release();
    }
    Instance->m_imageTextures.clear();

    for (auto& pair : Instance->m_fonts) {
        pair.second->release();
    }
    Instance->m_fonts.clear();

    for (auto& pair : Instance->m_sounds) {
        pair.second->release();
    }
    Instance->m_sounds.clear();

    Instance->m_textureReverse.clear();
    Instance->m_imageTextureReverse.clear();

    if (Instance)
    {
        delete Instance;
    }
}

AssetManager::~AssetManager()
{
    cleanUp();
}

std::string AssetManager::getAssetsPath() {
    // runtime override used by prefab editor
    if (!s_runtimeAssetsPath.empty()) {
        return s_runtimeAssetsPath;
    }

#ifdef ASSETS_PATH
    return ASSETS_PATH;
#endif

#ifdef __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle) {
        CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        if (resourceURL) {
            char path[PATH_MAX];
            if (CFURLGetFileSystemRepresentation(resourceURL, TRUE, (UInt8*)path, PATH_MAX)) {
                CFRelease(resourceURL);
                return std::string(path) + "/Assets/";
            }
            CFRelease(resourceURL);
        }
    }
#endif

#ifdef _WIN32
    char path[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        std::string fullPath(path);
        size_t pos = fullPath.find_last_of("\\/");
        if (pos != std::string::npos) {
            return fullPath.substr(0, pos) + "\\Assets\\";
        }
    }
#endif

    return "../../Assets/"; // fallback path for development
}

void AssetManager::setPixelsPerUnit(float pixelsPerUnit)
{
    if (pixelsPerUnit <= 0.0f)
    {
        Logger::error("AssetManager: pixelsPerUnit must be positive");
        return;
    }

    s_pixelsPerUnit = pixelsPerUnit;
}

auto AssetManager::getPixelsPerUnit() -> float
{
    return s_pixelsPerUnit;
}

void AssetManager::addTexture(const std::string& name, const std::string& path, const glm::i32 numFrames, const float pixelsPerUnit) {
    auto tex = std::make_shared<Sprout::Texture>();

    if (!tex->loadFromFile(getAssetsPath() + path, numFrames))
    {
        std::string fullPath = getAssetsPath() + path;
        throw std::runtime_error("Error loading texture file: " + fullPath);
    }

    tex->setPixelsPerUnit(pixelsPerUnit);
    tex->registerTexture();
    Instance->m_textures[name] = tex;
    Instance->m_textureReverse[tex.get()] = name;
    Instance->m_texturePaths[name] = path;
}

auto AssetManager::getTexture(const std::string& name) -> std::shared_ptr<Sprout::Texture> {
    auto it = Instance->m_textures.find(name);
    if (it == Instance->m_textures.end()) {
        throw std::runtime_error("Texture not found: " + name);
    }

    return it->second;
}

void AssetManager::addTileSet(const std::string& name, const std::string& path, size_t w, size_t h)
{
    std::vector<std::shared_ptr<Sprout::Texture>> tileset = Sprout::Texture::loadTileset(getAssetsPath() + path, w, h);
    if (tileset.empty())
    {
        throw std::runtime_error("Error loading tileset file: " + path);
    }
    Instance->m_tilesets[name] = tileset;
}

auto AssetManager::getTileSet(const std::string &name) -> std::vector<std::shared_ptr<Sprout::Texture>>&
{
    auto it = Instance->m_tilesets.find(name);
    if (it == Instance->m_tilesets.end()) {
        throw std::runtime_error("Tileset not found: " + name);
    }
    return it->second;
}

void AssetManager::addSound(const std::string &name, const std::string &path, bool loop)
{
    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;

    FMOD::System* system = AudioEngine::getSystem();
    FMOD_RESULT result = system->createSound((getAssetsPath() + path).c_str(), mode, nullptr, &sound);
    if (result != FMOD_OK)
    {
        throw std::runtime_error("Error loading sound file: " + path);
    }

    sound->setMode(mode);

    Instance->m_sounds[name] = sound;
}

auto AssetManager::getSound(const std::string &name) -> FMOD::Sound*
{
    auto it = Instance->m_sounds.find(name);
    if (it == Instance->m_sounds.end()) {
        throw std::runtime_error("Sound not found: " + name);
    }
    return it->second;
}

void AssetManager::addFont(const std::string &name, const std::string &path, float size)
{
    auto font = std::make_shared<Sprout::Font>();
    if (!font->loadFromFile(getAssetsPath() + path, size))
    {
        throw std::runtime_error("Error loading font file: " + path);
    }
    Instance->m_fonts[name] = font;
}

auto AssetManager::getFont(const std::string &name) -> std::shared_ptr<Sprout::Font>
{
    auto it = Instance->m_fonts.find(name);
    if (it == Instance->m_fonts.end()) {
        throw std::runtime_error("Font not found: " + name);
    }
    return it->second;
}

bool AssetManager::hasFont(const std::string& name)
{
    return Instance->m_fonts.find(name) != Instance->m_fonts.end();
}

auto AssetManager::getFontNames() -> std::vector<std::string>
{
    std::vector<std::string> names;
    names.reserve(Instance->m_fonts.size());
    for (const auto& [name, _] : Instance->m_fonts)
        names.push_back(name);
    std::sort(names.begin(), names.end());
    return names;
}

void AssetManager::addImageTexture(const std::string& name, const std::string& path, const float pixelsPerUnit)
{
    auto tex = std::make_shared<Sprout::Texture>();
    tex->setMode(Sprout::TextureMode::Independent);

    if (!tex->prepareFromFile(getAssetsPath() + path))
    {
        std::string fullPath = getAssetsPath() + path;
        throw std::runtime_error("Error preparing image texture file: " + fullPath);
    }
    tex->setPixelsPerUnit(pixelsPerUnit);
    Instance->m_imageTextures[name] = tex;
    Instance->m_imageTextureReverse[tex.get()] = name;
    Instance->m_imageTexturePaths[name] = path;
}

auto AssetManager::getImageTexture(const std::string& name) -> std::shared_ptr<Sprout::Texture>
{
    auto it = Instance->m_imageTextures.find(name);
    if (it == Instance->m_imageTextures.end()) {
        throw std::runtime_error("Image texture not found: " + name);
    }

    return it->second;
}

void AssetManager::setAssetsPath(const std::string& path) {
    s_runtimeAssetsPath = path;
}


bool AssetManager::hasTexture(const std::string& name) {
    return Instance->m_textures.count(name) > 0;
}

bool AssetManager::hasImageTexture(const std::string& name) {
    return Instance->m_imageTextures.count(name) > 0;
}

auto AssetManager::getTextureName(const std::shared_ptr<Sprout::Texture>& tex) -> std::string {
    auto it = Instance->m_textureReverse.find(tex.get());
    return (it != Instance->m_textureReverse.end()) ? it->second : "";
}

auto AssetManager::getImageTextureName(const std::shared_ptr<Sprout::Texture>& tex) -> std::string {
    auto it = Instance->m_imageTextureReverse.find(tex.get());
    return (it != Instance->m_imageTextureReverse.end()) ? it->second : "";
}

auto AssetManager::getImageTextureNames() -> std::vector<std::string> {
    std::vector<std::string> names;
    names.reserve(Instance->m_imageTextures.size());
    for (const auto& [name, _] : Instance->m_imageTextures) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

auto AssetManager::getTexturePath(const std::string& name) -> std::string {
    auto it = Instance->m_texturePaths.find(name);
    return (it != Instance->m_texturePaths.end()) ? it->second : "";
}

auto AssetManager::getImageTexturePath(const std::string& name) -> std::string {
    auto it = Instance->m_imageTexturePaths.find(name);
    return (it != Instance->m_imageTexturePaths.end()) ? it->second : "";
}

auto AssetManager::ensureImageTexture(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Texture> {
    if (hasImageTexture(name)) return getImageTexture(name);
    addImageTexture(name, path);
    return getImageTexture(name);
}

void AssetManager::registerTexture(const std::string& name, const std::string& path,
                                   Sprout::TextureMode mode, glm::i32 numFrames, const float pixelsPerUnit) {
    if (mode == Sprout::TextureMode::Atlas) {
        addTexture(name, path, numFrames, pixelsPerUnit);
    } else {
        addImageTexture(name, path, pixelsPerUnit);
    }
}

auto AssetManager::findTexture(const std::string& name) -> std::shared_ptr<Sprout::Texture> {
    // check atlas textures first
    auto it = Instance->m_textures.find(name);
    if (it != Instance->m_textures.end()) return it->second;
    // then independent textures
    auto it2 = Instance->m_imageTextures.find(name);
    if (it2 != Instance->m_imageTextures.end()) return it2->second;
    return nullptr;
}

auto AssetManager::scanAssetFiles(const std::vector<std::string>& extensions) -> std::vector<std::string> {
    std::vector<std::string> results;
    std::string assetsDir = getAssetsPath();
    if (assetsDir.empty()) return results;

    try {
        for (auto& entry : std::filesystem::recursive_directory_iterator(assetsDir)) {
            if (!entry.is_regular_file()) continue;
            std::string ext = entry.path().extension().string();
            std::string extLower = ext;
            for (auto& c : extLower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            for (const auto& wanted : extensions) {
                std::string wantedLower = wanted;
                for (auto& c : wantedLower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                if (extLower == wantedLower) {
                    std::string rel = std::filesystem::relative(entry.path(), assetsDir).string();
                    results.push_back(rel);
                    break;
                }
            }
        }
    } catch (...) {}

    std::sort(results.begin(), results.end());
    return results;
}

static bool hasRequiredManifestString(const nlohmann::json& entry,
                                      const std::string& fieldName,
                                      const std::string& manifestPath,
                                      const std::string& entryType)
{
    if (entry.contains(fieldName) && entry[fieldName].is_string() && !entry[fieldName].get<std::string>().empty())
    {
        return true;
    }

    Logger::error("AssetManager: " + manifestPath + " has " + entryType + " entry without required string field '" + fieldName + "'");
    return false;
}

void AssetManager::loadManifest(const std::string& manifestPath)
{
    const auto manifestJson = ManifestLoader::loadJson(manifestPath);
    loadManifest(manifestJson, manifestPath);
}

void AssetManager::loadManifest(const nlohmann::json& manifestJson, const std::string& sourceName)
{
    if (!manifestJson.is_object() || !manifestJson.contains("assets"))
    {
        return;
    }

    const auto& assets = manifestJson["assets"];
    if (!assets.is_object())
    {
        Logger::error("AssetManager: " + sourceName + " field 'assets' must be an object");
        return;
    }

    if (assets.contains("pixelsPerUnit"))
    {
        if (!assets["pixelsPerUnit"].is_number() || assets["pixelsPerUnit"].get<float>() <= 0.0f)
        {
            Logger::error("AssetManager: " + sourceName + " field 'assets.pixelsPerUnit' must be a positive number");
        }
        else
        {
            setPixelsPerUnit(assets["pixelsPerUnit"].get<float>());
        }
    }

    if (assets.contains("textures"))
    {
        if (!assets["textures"].is_array())
        {
            Logger::error("AssetManager: " + sourceName + " field 'assets.textures' must be an array");
        }
        else
        {
            for (const auto& texture : assets["textures"])
            {
                if (!texture.is_object())
                {
                    Logger::error("AssetManager: " + sourceName + " has a texture entry that is not an object");
                    continue;
                }

                if (!hasRequiredManifestString(texture, "name", sourceName, "texture") ||
                    !hasRequiredManifestString(texture, "path", sourceName, "texture"))
                {
                    continue;
                }

                const std::string name = texture["name"].get<std::string>();
                const std::string path = texture["path"].get<std::string>();
                if (texture.contains("mode") && !texture["mode"].is_string())
                {
                    Logger::error("AssetManager: " + sourceName + " texture '" + name + "' field 'mode' must be a string");
                    continue;
                }
                if (texture.contains("frames") &&
                    (!texture["frames"].is_number_integer() || texture["frames"].get<int>() <= 0))
                {
                    Logger::error("AssetManager: " + sourceName + " texture '" + name + "' field 'frames' must be a positive integer");
                    continue;
                }

                const std::string mode = texture.contains("mode") ? texture["mode"].get<std::string>() : "atlas";
                const glm::i32 frames = texture.contains("frames") ? texture["frames"].get<glm::i32>() : 1;
                float pixelsPerUnit = 0.0f;
                if (texture.contains("pixelsPerUnit"))
                {
                    if (!texture["pixelsPerUnit"].is_number() || texture["pixelsPerUnit"].get<float>() <= 0.0f)
                    {
                        Logger::error("AssetManager: " + sourceName + " texture '" + name + "' field 'pixelsPerUnit' must be a positive number");
                    }
                    else
                    {
                        pixelsPerUnit = texture["pixelsPerUnit"].get<float>();
                    }
                }

                if (mode == "atlas")
                {
                    addTexture(name, path, frames, pixelsPerUnit);
                }
                else if (mode == "image")
                {
                    addImageTexture(name, path, pixelsPerUnit);
                }
                else
                {
                    Logger::error("AssetManager: " + sourceName + " texture '" + name + "' has unknown mode '" + mode + "'");
                }
            }
        }
    }

    if (assets.contains("fonts"))
    {
        if (!assets["fonts"].is_array())
        {
            Logger::error("AssetManager: " + sourceName + " field 'assets.fonts' must be an array");
        }
        else
        {
            for (const auto& font : assets["fonts"])
            {
                if (!font.is_object())
                {
                    Logger::error("AssetManager: " + sourceName + " has a font entry that is not an object");
                    continue;
                }

                if (!hasRequiredManifestString(font, "name", sourceName, "font") ||
                    !hasRequiredManifestString(font, "path", sourceName, "font"))
                {
                    continue;
                }

                if (!font.contains("size") || !font["size"].is_number())
                {
                    Logger::error("AssetManager: " + sourceName + " font '" + font["name"].get<std::string>() + "' is missing numeric field 'size'");
                    continue;
                }

                addFont(font["name"].get<std::string>(), font["path"].get<std::string>(), font["size"].get<float>());
            }
        }
    }

    if (assets.contains("sounds"))
    {
        if (!assets["sounds"].is_array())
        {
            Logger::error("AssetManager: " + sourceName + " field 'assets.sounds' must be an array");
        }
        else
        {
            for (const auto& sound : assets["sounds"])
            {
                if (!sound.is_object())
                {
                    Logger::error("AssetManager: " + sourceName + " has a sound entry that is not an object");
                    continue;
                }

                if (!hasRequiredManifestString(sound, "name", sourceName, "sound") ||
                    !hasRequiredManifestString(sound, "path", sourceName, "sound"))
                {
                    continue;
                }

                const std::string name = sound["name"].get<std::string>();
                if (sound.contains("loop") && !sound["loop"].is_boolean())
                {
                    Logger::error("AssetManager: " + sourceName + " sound '" + name + "' field 'loop' must be a boolean");
                    continue;
                }

                addSound(name,
                         sound["path"].get<std::string>(),
                         sound.contains("loop") ? sound["loop"].get<bool>() : false);
            }
        }
    }

    if (assets.contains("tilesets"))
    {
        if (!assets["tilesets"].is_array())
        {
            Logger::error("AssetManager: " + sourceName + " field 'assets.tilesets' must be an array");
        }
        else
        {
            for (const auto& tileset : assets["tilesets"])
            {
                if (!tileset.is_object())
                {
                    Logger::error("AssetManager: " + sourceName + " has a tileset entry that is not an object");
                    continue;
                }

                if (!hasRequiredManifestString(tileset, "name", sourceName, "tileset") ||
                    !hasRequiredManifestString(tileset, "path", sourceName, "tileset"))
                {
                    continue;
                }

                if (!tileset.contains("tileWidth") || !tileset["tileWidth"].is_number_integer() ||
                    !tileset.contains("tileHeight") || !tileset["tileHeight"].is_number_integer() ||
                    tileset["tileWidth"].get<int>() <= 0 || tileset["tileHeight"].get<int>() <= 0)
                {
                    Logger::error("AssetManager: " + sourceName + " tileset '" + tileset["name"].get<std::string>() + "' requires numeric 'tileWidth' and 'tileHeight'");
                    continue;
                }

                addTileSet(tileset["name"].get<std::string>(),
                           tileset["path"].get<std::string>(),
                           static_cast<size_t>(tileset["tileWidth"].get<int>()),
                           static_cast<size_t>(tileset["tileHeight"].get<int>()));
            }
        }
    }
}
