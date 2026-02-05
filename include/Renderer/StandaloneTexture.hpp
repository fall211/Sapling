//
//  StandaloneTexture.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

#include <glm/glm.hpp>
#include "sokol/sokol_gfx.h"

#include <cstddef>
#include <string>
#include <memory>

namespace Sprout
{
    class StandaloneTexture
    {
        public:
            StandaloneTexture() = default;
            ~StandaloneTexture();
            
            /*
                * Loads a texture from a file without adding it to the atlas
                * @param path The path to the texture file
                * @return True if the texture was loaded successfully
            */
            auto loadFromFile(const std::string& path) -> bool;
            
            /*
                * Prepares a texture from a file but delays GPU loading until ensureLoaded is called
                * @param path The path to the texture file
                * @return True if the file was found
            */
            auto prepareFromFile(const std::string& path) -> bool;
            
            /*
                * Ensures the texture is loaded into GPU memory
                * Call this after Sokol GFX is initialized
                * @return True if the texture was loaded successfully
            */
            auto ensureLoaded() -> bool;
            
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
                * Gets the sokol image handle
                * @return The sokol image handle
            */
            auto getImageHandle() -> sg_image;
            
            /*
                * Loads a texture from memory
                * @param data The pixel data of the texture
                * @param width The width of the texture
                * @param height The height of the texture
                * @return True if the texture was loaded successfully
            */
            auto loadFromMemory(unsigned char* data, glm::i32 width, glm::i32 height) -> bool;
            
            /*
                * Releases the texture resources
            */
            void release();
            
        private:
            glm::i32 m_width = 0;
            glm::i32 m_height = 0;
            sg_image m_image = {};
            bool m_isLoaded = false;
            bool m_isPrepared = false;
            std::string m_filePath;
            unsigned char* m_pixels = nullptr;
    };
}