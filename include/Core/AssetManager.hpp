//
//  AssetManager.hpp
//  SaplingEngine, Seedbank Asset Manager
//

#pragma once

#include "Renderer/Texture.hpp"
#include "Renderer/Font.hpp"

#include "glm/glm.hpp"
#include "fmod.hpp"

#include <map>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>

#include <nlohmann/json_fwd.hpp>

class AudioEngine;

class AssetManager {
    std::unordered_map<std::string, std::shared_ptr<Sprout::Texture>> m_textures = std::unordered_map<std::string, std::shared_ptr<Sprout::Texture>>();
    std::unordered_map<std::string, std::shared_ptr<Sprout::Font>> m_fonts = std::unordered_map<std::string, std::shared_ptr<Sprout::Font>>();

    std::unordered_map<std::string, std::vector<std::shared_ptr<Sprout::Texture>>> m_tilesets = std::unordered_map<std::string, std::vector<std::shared_ptr<Sprout::Texture>>>();

    std::unordered_map<std::string, FMOD::Sound*> m_sounds = std::unordered_map<std::string, FMOD::Sound*>();

    std::unordered_map<std::string, std::shared_ptr<Sprout::Texture>> m_imageTextures = std::unordered_map<std::string, std::shared_ptr<Sprout::Texture>>();

    std::unordered_map<std::string, std::string> m_texturePaths;
    std::unordered_map<std::string, std::string> m_imageTexturePaths;

    std::unordered_map<const Sprout::Texture*, std::string> m_textureReverse;
    std::unordered_map<const Sprout::Texture*, std::string> m_imageTextureReverse;


    static AssetManager* Instance;
    static std::string s_runtimeAssetsPath;
    static float s_pixelsPerUnit;

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
    static void setPixelsPerUnit(float pixelsPerUnit);
    static auto getPixelsPerUnit() -> float;

    /*
        * Adds an atlas-mode texture to the asset manager.
        * The texture will be packed into the shared atlas for batched 2D rendering.
        * @param name The name of the texture
        * @param path The path to the texture
        * @param numFrames The number of frames in the texture (if animated)
    */
    static void addTexture(const std::string& name,
        const std::string& path,
        glm::i32 numFrames = 1,
        float pixelsPerUnit = 0.0f
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
    static void addImageTexture(const std::string& name, const std::string& path, float pixelsPerUnit = 0.0f);

    /*
        * Gets the independent-mode texture with the given name.
        * @param name The name of the texture
        * @return The texture with the given name
    */
    static auto getImageTexture(const std::string& name) -> std::shared_ptr<Sprout::Texture>;

    static auto getTextureName(const std::shared_ptr<Sprout::Texture>& tex) -> std::string;
    static auto getImageTextureName(const std::shared_ptr<Sprout::Texture>& tex) -> std::string;

    static void setAssetsPath(const std::string& path);

    static bool hasTexture(const std::string& name);
    static bool hasImageTexture(const std::string& name);

    static auto ensureImageTexture(const std::string& name, const std::string& path) -> std::shared_ptr<Sprout::Texture>;

    static auto getTexturePath(const std::string& name) -> std::string;
    static auto getImageTexturePath(const std::string& name) -> std::string;

    static auto getImageTextureNames() -> std::vector<std::string>;

    static void registerTexture(const std::string& name, const std::string& path,
                                Sprout::TextureMode mode = Sprout::TextureMode::Atlas,
                                glm::i32 numFrames = 1,
                                float pixelsPerUnit = 0.0f);

    static auto findTexture(const std::string& name) -> std::shared_ptr<Sprout::Texture>;

    static auto scanAssetFiles(const std::vector<std::string>& extensions) -> std::vector<std::string>;

    static void loadManifest(const std::string& manifestPath);
    static void loadManifest(const nlohmann::json& manifestJson, const std::string& sourceName = "manifest");
};
