//
//  AssetManager.hpp
//  SaplingEngine, Seedbank Asset Manager
//

#pragma once

#include "Renderer/Texture.hpp"
#include "Renderer/StandaloneTexture.hpp"
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
    std::unordered_map<std::string, std::shared_ptr<Sprout::StandaloneTexture>> m_standaloneTextures = std::unordered_map<std::string, std::shared_ptr<Sprout::StandaloneTexture>>();
    std::unordered_map<std::string, std::shared_ptr<Sprout::Font>> m_fonts = std::unordered_map<std::string, std::shared_ptr<Sprout::Font>>();
    
    std::unordered_map<std::string, std::vector<std::shared_ptr<Sprout::Texture>>> m_tilesets = std::unordered_map<std::string, std::vector<std::shared_ptr<Sprout::Texture>>>();
    
    std::unordered_map<std::string, FMOD::Sound*> m_sounds = std::unordered_map<std::string, FMOD::Sound*>();
    
    
    
    static AssetManager* Instance;

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
        * Adds a texture to the asset manager
        * @param name The name of the texture
        * @param path The path to the texture
        * @param numFrames The number of frames in the texture (if animated)
    */
    static void addTexture(const std::string& name, 
        const std::string& path,
        glm::i32 numFrames = 1
    );

    /*
        * Gets the texture with the given name
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
    
    /*
        * Adds a standalone texture to the asset manager
        * @param name The name of the texture
        * @param path The path to the texture
    */
    static void addStandaloneTexture(const std::string& name, const std::string& path);
    
    /*
        * Gets the standalone texture with the given name
        * @param name The name of the texture
        * @return The standalone texture with the given name
    */
    static auto getStandaloneTexture(const std::string& name) -> std::shared_ptr<Sprout::StandaloneTexture>;
    
};

