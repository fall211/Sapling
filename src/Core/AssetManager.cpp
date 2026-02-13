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
std::string AssetManager::s_runtimeAssetsPath = "";

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

    for (auto& pair : Instance->m_meshes) {
        pair.second->release();
    }
    Instance->m_meshes.clear();

    Instance->m_materials.clear();
    Instance->m_skeletons.clear();
    Instance->m_animationClips.clear();

    Instance->m_textureReverse.clear();
    Instance->m_imageTextureReverse.clear();
    Instance->m_meshReverse.clear();
    Instance->m_skeletonReverse.clear();
    Instance->m_animClipReverse.clear();
    Instance->m_materialReverse.clear();
    Instance->m_materialPaths.clear();

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

void AssetManager::addTexture(const std::string& name, const std::string& path, const glm::i32 numFrames) {
    auto tex = std::make_shared<Sprout::Texture>();

    if (!tex->loadFromFile(getAssetsPath() + path, numFrames))
    {
        std::string fullPath = getAssetsPath() + path;
        throw std::runtime_error("Error loading texture file: " + fullPath);
    }

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

void AssetManager::addImageTexture(const std::string& name, const std::string& path)
{
    auto tex = std::make_shared<Sprout::Texture>();
    tex->setMode(Sprout::TextureMode::Independent);

    if (!tex->prepareFromFile(getAssetsPath() + path))
    {
        std::string fullPath = getAssetsPath() + path;
        throw std::runtime_error("Error preparing image texture file: " + fullPath);
    }
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

void AssetManager::addMesh(const std::string& name, const std::string& filepath)
{
    auto mesh = std::make_shared<Sprout::Mesh>();
    std::string full_path = getAssetsPath() + filepath;
    std::string ext = std::filesystem::path(filepath).extension().string();
    for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    bool ok = false;
    if (ext == ".fbx") {
        ok = mesh->loadFBX(filepath);
    } else {
        ok = mesh->loadOBJ(filepath);
    }
    if (!ok) {
        throw std::runtime_error("Error loading mesh file: " + full_path);
    }
    Instance->m_meshes[name] = mesh;
    Instance->m_meshReverse[mesh.get()] = name;
    Instance->m_meshPaths[name] = filepath;
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

bool AssetManager::hasMesh(const std::string& name) {
    return Instance->m_meshes.count(name) > 0;
}

bool AssetManager::hasSkeleton(const std::string& name) {
    return Instance->m_skeletons.count(name) > 0;
}

bool AssetManager::hasAnimationClip(const std::string& name) {
    return Instance->m_animationClips.count(name) > 0;
}

auto AssetManager::getTextureName(const std::shared_ptr<Sprout::Texture>& tex) -> std::string {
    auto it = Instance->m_textureReverse.find(tex.get());
    return (it != Instance->m_textureReverse.end()) ? it->second : "";
}

auto AssetManager::getImageTextureName(const std::shared_ptr<Sprout::Texture>& tex) -> std::string {
    auto it = Instance->m_imageTextureReverse.find(tex.get());
    return (it != Instance->m_imageTextureReverse.end()) ? it->second : "";
}

auto AssetManager::getMeshName(const std::shared_ptr<Sprout::Mesh>& mesh) -> std::string {
    auto it = Instance->m_meshReverse.find(mesh.get());
    return (it != Instance->m_meshReverse.end()) ? it->second : "";
}

auto AssetManager::getSkeletonName(const std::shared_ptr<Sprout::Skeleton>& skel) -> std::string {
    auto it = Instance->m_skeletonReverse.find(skel.get());
    return (it != Instance->m_skeletonReverse.end()) ? it->second : "";
}

auto AssetManager::getAnimationClipName(const std::shared_ptr<Sprout::AnimationClip>& clip) -> std::string {
    auto it = Instance->m_animClipReverse.find(clip.get());
    return (it != Instance->m_animClipReverse.end()) ? it->second : "";
}

void AssetManager::addMesh(const std::string& name, std::shared_ptr<Sprout::Mesh> mesh)
{
    Instance->m_meshReverse[mesh.get()] = name;
    Instance->m_meshes[name] = std::move(mesh);
}

auto AssetManager::getMesh(const std::string& name) -> std::shared_ptr<Sprout::Mesh>
{
    auto it = Instance->m_meshes.find(name);
    if (it == Instance->m_meshes.end()) {
        throw std::runtime_error("Mesh not found: " + name);
    }
    return it->second;
}

void AssetManager::addMaterial(const std::string& name, std::shared_ptr<Sprout::Material> material)
{
    Instance->m_materials[name] = material;
    Instance->m_materialReverse[material.get()] = name;
}

void AssetManager::addMaterial(const std::string& name, const std::string& path)
{
    auto material = std::make_shared<Sprout::Material>();
    if (!material->loadFromFile(path)) {
        throw std::runtime_error("Failed to load material: " + path);
    }
    Instance->m_materials[name] = material;
    Instance->m_materialPaths[name] = path;
    Instance->m_materialReverse[material.get()] = name;
}

auto AssetManager::getMaterial(const std::string& name) -> std::shared_ptr<Sprout::Material>
{
    auto it = Instance->m_materials.find(name);
    if (it == Instance->m_materials.end()) {
        throw std::runtime_error("Material not found: " + name);
    }
    return it->second;
}

void AssetManager::addSkeleton(const std::string& name, const std::string& filepath)
{
    auto skeleton = std::make_shared<Sprout::Skeleton>();
    if (!skeleton->loadFromFBX(filepath)) {
        throw std::runtime_error("Error loading skeleton: " + filepath);
    }
    Instance->m_skeletons[name] = skeleton;
    Instance->m_skeletonReverse[skeleton.get()] = name;
    Instance->m_skeletonPaths[name] = filepath;
}

auto AssetManager::getSkeleton(const std::string& name) -> std::shared_ptr<Sprout::Skeleton>
{
    auto it = Instance->m_skeletons.find(name);
    if (it == Instance->m_skeletons.end()) {
        throw std::runtime_error("Skeleton not found: " + name);
    }
    return it->second;
}

void AssetManager::addAnimationClip(const std::string& name, const std::string& filepath,
                                    const std::string& skeletonName, const std::string& clipName)
{
    auto skeleton = getSkeleton(skeletonName);
    auto clip = std::make_shared<Sprout::AnimationClip>();
    if (!clip->loadFromFBX(filepath, *skeleton, clipName)) {
        throw std::runtime_error("Error loading animation clip: " + filepath);
    }
    Instance->m_animationClips[name] = clip;
    Instance->m_animClipReverse[clip.get()] = name;
    Instance->m_animClipPaths[name] = filepath;
}

auto AssetManager::getAnimationClip(const std::string& name) -> std::shared_ptr<Sprout::AnimationClip>
{
    auto it = Instance->m_animationClips.find(name);
    if (it == Instance->m_animationClips.end()) {
        throw std::runtime_error("Animation clip not found: " + name);
    }
    return it->second;
}

void AssetManager::addSkinnedMesh(const std::string& name, const std::string& filepath,
                                  const std::string& skeletonName)
{
    auto skeleton = getSkeleton(skeletonName);
    auto mesh = std::make_shared<Sprout::Mesh>();
    if (!mesh->loadFBX(filepath, skeleton.get())) {
        throw std::runtime_error("Error loading skinned mesh: " + filepath);
    }
    Instance->m_meshes[name] = mesh;
    Instance->m_meshReverse[mesh.get()] = name;
}

auto AssetManager::getMeshNames() -> std::vector<std::string> {
    std::vector<std::string> names;
    names.reserve(Instance->m_meshes.size());
    for (const auto& [name, _] : Instance->m_meshes) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

auto AssetManager::getSkeletonNames() -> std::vector<std::string> {
    std::vector<std::string> names;
    names.reserve(Instance->m_skeletons.size());
    for (const auto& [name, _] : Instance->m_skeletons) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

auto AssetManager::getAnimationClipNames() -> std::vector<std::string> {
    std::vector<std::string> names;
    names.reserve(Instance->m_animationClips.size());
    for (const auto& [name, _] : Instance->m_animationClips) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
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

bool AssetManager::hasMaterial(const std::string& name) {
    return Instance->m_materials.count(name) > 0;
}

auto AssetManager::ensureMaterial(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Material> {
    if (hasMaterial(name)) return getMaterial(name);
    addMaterial(name, path);
    return getMaterial(name);
}

auto AssetManager::getMaterialName(const std::shared_ptr<Sprout::Material>& mat) -> std::string {
    if (!mat) return "";
    auto it = Instance->m_materialReverse.find(mat.get());
    return (it != Instance->m_materialReverse.end()) ? it->second : "";
}

auto AssetManager::getMaterialPath(const std::string& name) -> std::string {
    auto it = Instance->m_materialPaths.find(name);
    return (it != Instance->m_materialPaths.end()) ? it->second : "";
}

auto AssetManager::getTexturePath(const std::string& name) -> std::string {
    auto it = Instance->m_texturePaths.find(name);
    return (it != Instance->m_texturePaths.end()) ? it->second : "";
}

auto AssetManager::getImageTexturePath(const std::string& name) -> std::string {
    auto it = Instance->m_imageTexturePaths.find(name);
    return (it != Instance->m_imageTexturePaths.end()) ? it->second : "";
}

auto AssetManager::getMeshPath(const std::string& name) -> std::string {
    auto it = Instance->m_meshPaths.find(name);
    return (it != Instance->m_meshPaths.end()) ? it->second : "";
}

auto AssetManager::getSkeletonPath(const std::string& name) -> std::string {
    auto it = Instance->m_skeletonPaths.find(name);
    return (it != Instance->m_skeletonPaths.end()) ? it->second : "";
}

auto AssetManager::getAnimationClipPath(const std::string& name) -> std::string {
    auto it = Instance->m_animClipPaths.find(name);
    return (it != Instance->m_animClipPaths.end()) ? it->second : "";
}



auto AssetManager::ensureMesh(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Mesh> {
    if (hasMesh(name)) return getMesh(name);
    addMesh(name, path);
    return getMesh(name);
}

auto AssetManager::ensureImageTexture(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Texture> {
    if (hasImageTexture(name)) return getImageTexture(name);
    addImageTexture(name, path);
    return getImageTexture(name);
}

auto AssetManager::ensureSkeleton(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Skeleton> {
    if (hasSkeleton(name)) return getSkeleton(name);
    addSkeleton(name, path);
    return getSkeleton(name);
}

auto AssetManager::ensureAnimationClip(const std::string& name, const std::string& path,
                                       const std::string& skeletonName,
                                       const std::string& clipName) -> std::shared_ptr<Sprout::AnimationClip> {
    if (hasAnimationClip(name)) return getAnimationClip(name);
    addAnimationClip(name, path, skeletonName, clipName);
    return getAnimationClip(name);
}

auto AssetManager::ensureSkinnedMesh(const std::string& name, const std::string& path,
                                     const std::string& skeletonName) -> std::shared_ptr<Sprout::Mesh> {
    if (hasMesh(name)) return getMesh(name);
    addSkinnedMesh(name, path, skeletonName);
    return getMesh(name);
}

void AssetManager::registerTexture(const std::string& name, const std::string& path,
                                   Sprout::TextureMode mode, glm::i32 numFrames) {
    if (mode == Sprout::TextureMode::Atlas) {
        addTexture(name, path, numFrames);
    } else {
        addImageTexture(name, path);
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
