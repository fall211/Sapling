//
//  AssetManager.hpp
//  SaplingEngine, Seedbank Asset Manager
//

#pragma once

#include "Renderer/Texture.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Skeleton.hpp"
#include "Renderer/Animation.hpp"
#include "Renderer/Font.hpp"

#include "glm/glm.hpp"
#include "fmod.hpp"

#include <map>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>

class AudioEngine;

class AssetManager {
    std::unordered_map<std::string, std::shared_ptr<Sprout::Texture>> m_textures = std::unordered_map<std::string, std::shared_ptr<Sprout::Texture>>();
    std::unordered_map<std::string, std::shared_ptr<Sprout::Font>> m_fonts = std::unordered_map<std::string, std::shared_ptr<Sprout::Font>>();

    std::unordered_map<std::string, std::vector<std::shared_ptr<Sprout::Texture>>> m_tilesets = std::unordered_map<std::string, std::vector<std::shared_ptr<Sprout::Texture>>>();

    std::unordered_map<std::string, FMOD::Sound*> m_sounds = std::unordered_map<std::string, FMOD::Sound*>();

    std::unordered_map<std::string, std::shared_ptr<Sprout::Mesh>> m_meshes;
    std::unordered_map<std::string, std::shared_ptr<Sprout::Material>> m_materials;
    std::unordered_map<std::string, std::shared_ptr<Sprout::Skeleton>> m_skeletons;
    std::unordered_map<std::string, std::shared_ptr<Sprout::AnimationClip>> m_animationClips;

    std::unordered_map<std::string, std::shared_ptr<Sprout::Texture>> m_imageTextures = std::unordered_map<std::string, std::shared_ptr<Sprout::Texture>>();

    std::unordered_map<std::string, std::string> m_texturePaths;
    std::unordered_map<std::string, std::string> m_imageTexturePaths;
    std::unordered_map<std::string, std::string> m_meshPaths;
    std::unordered_map<std::string, std::string> m_skeletonPaths;
    std::unordered_map<std::string, std::string> m_animClipPaths;
    std::unordered_map<std::string, std::string> m_materialPaths;

    std::unordered_map<const Sprout::Texture*, std::string> m_textureReverse;
    std::unordered_map<const Sprout::Texture*, std::string> m_imageTextureReverse;
    std::unordered_map<const Sprout::Mesh*, std::string> m_meshReverse;
    std::unordered_map<const Sprout::Skeleton*, std::string> m_skeletonReverse;
    std::unordered_map<const Sprout::AnimationClip*, std::string> m_animClipReverse;
    std::unordered_map<const Sprout::Material*, std::string> m_materialReverse;


    static AssetManager* Instance;
    static std::string s_runtimeAssetsPath;

    AssetManager() = default;
    ~AssetManager();

public:
    static void initialize();
    static void cleanUp();
    static AssetManager* getInstance()
    {
        if (Instance == nullptr) {
            Instance = new AssetManager();
        }
        return Instance;
    }

    /*
        * Gets the assets path used by the game.
        * @return The assets path
    */
    static std::string getAssetsPath();
    /*
        * Adds an atlas-mode texture to the asset manager.
        * The texture will be packed into the shared atlas for batched 2D rendering.
        * @param name The name of the texture
        * @param path The path to the texture
        * @param numFrames The number of frames in the texture (if animated)
    */
    static void addTexture(const std::string& name,
        const std::string& path,
        glm::i32 numFrames = 1
    );

    /*
        * Gets the atlas-mode texture with the given name.
        * @param name The name of the texture
        * @return The texture with the given name
    */
    static auto getTexture(const std::string& name) -> std::shared_ptr<Sprout::Texture>;

    /*
        * Adds a tileset to the asset manager
        * @param name The name of the tileset
        * @param path The path to the tileset
        * @param w The width of the tileset
        * @param h The height of the tileset
    */
    static void addTileSet(const std::string& name, const std::string& path, size_t w, size_t h);

    /*
        * Gets the tileset with the given name
        * @param name The name of the tileset
        * @return Pointer to the tileset with the given name
    */
    static auto getTileSet(const std::string& name) -> std::vector<std::shared_ptr<Sprout::Texture>>&;

    /*
        * Adds a sound to the asset manager
        * @param name The name of the sound
        * @param path The path to the sound
    */
    static void addSound(const std::string& name, const std::string& path, bool loop = false);

    /*
        * Gets the sound with the given name
        * @param name The name of the sound
        * @return Pointer to the sound with the given name
    */
    static auto getSound(const std::string& name) -> FMOD::Sound*;

    /*
        * Adds a font to the asset manager
        * @param name The name of the font
        * @param path The path to the font
        * @param size The size of the font
    */
    static void addFont(const std::string& name, const std::string& path, float size);

    /*
        * Gets the font with the given name
        * @param name The name of the font
        * @return Pointer to the font with the given name
    */
    static auto getFont(const std::string& name) -> std::shared_ptr<Sprout::Font>;
    static bool hasFont(const std::string& name);
    static auto getFontNames() -> std::vector<std::string>;

    /*
        * Adds an independent-mode (non-atlas) texture to the asset manager.
        * Used for backgrounds, splash screens, 3D diffuse textures, etc.
        * The texture gets its own GPU image and is not packed into the atlas.
        * @param name The name of the texture
        * @param path The path to the texture
    */
    static void addImageTexture(const std::string& name, const std::string& path);

    /*
        * Gets the independent-mode texture with the given name.
        * @param name The name of the texture
        * @return The texture with the given name
    */
    static auto getImageTexture(const std::string& name) -> std::shared_ptr<Sprout::Texture>;

    static void addMesh(const std::string& name, const std::string& filepath);
    static void addMesh(const std::string& name, std::shared_ptr<Sprout::Mesh> mesh);
    static auto getMesh(const std::string& name) -> std::shared_ptr<Sprout::Mesh>;

    static void addMaterial(const std::string& name, std::shared_ptr<Sprout::Material> material);
    static void addMaterial(const std::string& name, const std::string& path);
    static auto getMaterial(const std::string& name) -> std::shared_ptr<Sprout::Material>;
    static auto ensureMaterial(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Material>;

    static void addSkeleton(const std::string& name, const std::string& filepath);
    static auto getSkeleton(const std::string& name) -> std::shared_ptr<Sprout::Skeleton>;

    static void addAnimationClip(const std::string& name, const std::string& filepath,
                                 const std::string& skeletonName, const std::string& clipName = "");
    static auto getAnimationClip(const std::string& name) -> std::shared_ptr<Sprout::AnimationClip>;

    static void addSkinnedMesh(const std::string& name, const std::string& filepath,
                               const std::string& skeletonName);

    static auto getTextureName(const std::shared_ptr<Sprout::Texture>& tex) -> std::string;
    static auto getImageTextureName(const std::shared_ptr<Sprout::Texture>& tex) -> std::string;
    static auto getMeshName(const std::shared_ptr<Sprout::Mesh>& mesh) -> std::string;
    static auto getSkeletonName(const std::shared_ptr<Sprout::Skeleton>& skel) -> std::string;
    static auto getAnimationClipName(const std::shared_ptr<Sprout::AnimationClip>& clip) -> std::string;
    static auto getMaterialName(const std::shared_ptr<Sprout::Material>& mat) -> std::string;

    static void setAssetsPath(const std::string& path);

    static bool hasTexture(const std::string& name);
    static bool hasImageTexture(const std::string& name);
    static bool hasMesh(const std::string& name);
    static bool hasSkeleton(const std::string& name);
    static bool hasAnimationClip(const std::string& name);
    static bool hasMaterial(const std::string& name);

    static auto ensureMesh(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Mesh>;
    static auto ensureImageTexture(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Texture>;
    static auto ensureSkeleton(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Skeleton>;
    static auto ensureAnimationClip(const std::string& name, const std::string& path,
                                    const std::string& skeletonName,
                                    const std::string& clipName = "") -> std::shared_ptr<Sprout::AnimationClip>;
    static auto ensureSkinnedMesh(const std::string& name, const std::string& path,
                                  const std::string& skeletonName) -> std::shared_ptr<Sprout::Mesh>;

    static auto getTexturePath(const std::string& name) -> std::string;
    static auto getImageTexturePath(const std::string& name) -> std::string;
    static auto getMeshPath(const std::string& name) -> std::string;
    static auto getSkeletonPath(const std::string& name) -> std::string;
    static auto getAnimationClipPath(const std::string& name) -> std::string;
    static auto getMaterialPath(const std::string& name) -> std::string;

    static auto getMeshNames() -> std::vector<std::string>;
    static auto getSkeletonNames() -> std::vector<std::string>;
    static auto getAnimationClipNames() -> std::vector<std::string>;
    static auto getImageTextureNames() -> std::vector<std::string>;

    static void registerTexture(const std::string& name, const std::string& path,
                                Sprout::TextureMode mode = Sprout::TextureMode::Atlas,
                                glm::i32 numFrames = 1);

    static auto findTexture(const std::string& name) -> std::shared_ptr<Sprout::Texture>;

    static auto scanAssetFiles(const std::vector<std::string>& extensions) -> std::vector<std::string>;
};
