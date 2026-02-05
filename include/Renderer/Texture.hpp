//
//  Texture.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <string>
#include <vector>
#include <memory>

namespace Sprout
{
    
    class Texture
    {
        public:
            Texture() = default;
            ~Texture();
            
            /*
                * Loads a texture from a file
                * @param path The path to the texture file
                * @param numFrames The number of frames in the texture (if animated)
                * @return True if the texture was loaded successfully
            */
            auto loadFromFile(const std::string& path, glm::i32 numFrames = 1) -> bool;
            

            /*
                * Gets the pixel data of the texture
                * @return The pixel data of the texture
            */
            auto getPixels() -> unsigned char*;
            
            /*
                * Gets the size of the texture
                * @return The size of the texture
            */
            auto getSize() -> glm::vec2;
            
            /*
                * Gets the width of the texture
                * @return The width of the texture
            */
            auto getWidth() -> glm::i32;
            
            /*
                * Gets the height of the texture
                * @return The height of the texture
            */
            auto getHeight() -> glm::i32;
            
            /*
                * Gets the UV coordinates of the texture in the atlas
                * @return The UV coordinates of the texture in the atlas
            */
            auto getAtlasUVs() -> glm::vec4;
            
            /*
                * Sets the UV coordinates of the texture in the atlas
                * @param uvs The UV coordinates of the texture in the atlas
            */
            auto setAtlasUVs(glm::vec4 uvs) -> void;
            
            /*
                * Gets the size of a frame in the texture
                * @return The size of a frame in the texture
            */
            auto getFrameSize() -> glm::vec2;
            
            /*
                * Gets the number of frames in the texture
                * @return The number of frames in the texture
            */
            auto getNumFrames() -> glm::i32;
            
            /*
                * Registers the texture with the renderer (adds it to the atlas)
            */
            auto registerTexture() -> void;
            
            /*
                * Loads a tileset from a file
                * @param path The path to the tileset file
                * @param tileWidth The width of each tile in the tileset
                * @param tileHeight The height of each tile in the tileset
                * @return A vector of textures representing the tiles in the tileset
            */
            static auto loadTileset(const std::string& path, const size_t tileWidth, const size_t tileHeight) -> std::vector<std::shared_ptr<Texture>>;
            
            /*
                * Loads a texture from memory
                * @param data The pixel data of the texture
                * @param width The width of the texture
                * @param height The height of the texture
                * @param numFrames The number of frames in the texture (if animated)
                * @return True if the texture was loaded successfully
            */
            auto loadFromMemory(unsigned char* data, glm::i32 width, glm::i32 height, glm::i32 numFrames = 1) -> bool;
            
            void release();
            
        private:
            glm::i32 m_width, m_height;
            glm::vec4 m_atlas_uvs = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            unsigned char* m_pixels;
            
            // animated sprites data
            glm::i32 m_frameWidth, m_frameHeight, m_numFrames;
    };
    
}

