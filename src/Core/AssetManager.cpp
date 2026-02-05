//
//  AssetManager.cpp
//  SaplingEngine, Seedbank Asset Manager
//

#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Renderer/Font.hpp"
#include "Utility/Debug.hpp"

#include "fmod_common.h"

#include <filesystem>
#include <iostream>
#include <string>

AssetManager* AssetManager::Instance = nullptr;

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
}

void AssetManager::cleanUp()
{   
    for (auto& pair : Instance->m_textures) {
        pair.second->release();
    }
    Instance->m_textures.clear();
    
    for (auto& pair : Instance->m_standaloneTextures) {
        pair.second->release();
    }
    Instance->m_standaloneTextures.clear();
    
    for (auto& pair : Instance->m_fonts) {
        pair.second->release();
    }
    Instance->m_fonts.clear();
    
    for (auto& pair : Instance->m_sounds) {
        pair.second->release();
    }
    Instance->m_sounds.clear();
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

    return "../../GameContent/Assets/"; // fallback path for development
}

void AssetManager::addTexture(const std::string& name, const std::string& path, const glm::i32 numFrames) {
    auto tex = std::make_shared<Sprout::Texture>();
    
    if (!tex->loadFromFile(getAssetsPath() + path, numFrames)) 
    {        
        std::string fullPath = getAssetsPath() + path;
        throw std::runtime_error("Error loading texture file: " + fullPath);
    }
    Instance->m_textures[name] = tex;
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
    
    sound->setMode(FMOD_LOOP_NORMAL);
    
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

void AssetManager::addStandaloneTexture(const std::string& name, const std::string& path)
{
    auto tex = std::make_shared<Sprout::StandaloneTexture>();
    
    if (!tex->prepareFromFile(getAssetsPath() + path)) 
    {        
        std::string fullPath = getAssetsPath() + path;
        throw std::runtime_error("Error preparing standalone texture file: " + fullPath);
    }
    Instance->m_standaloneTextures[name] = tex;
}

auto AssetManager::getStandaloneTexture(const std::string& name) -> std::shared_ptr<Sprout::StandaloneTexture>
{
    auto it = Instance->m_standaloneTextures.find(name);
    if (it == Instance->m_standaloneTextures.end()) {
        throw std::runtime_error("Standalone texture not found: " + name);
    }
    
    return it->second;
}
