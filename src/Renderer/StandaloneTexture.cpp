//
//  StandaloneTexture.cpp
//  Sapling Engine, Sprout Renderer
//

#include "Renderer/StandaloneTexture.hpp"
#include "stb/stb_image.h"
#include <iostream>

namespace Sprout
{
    StandaloneTexture::~StandaloneTexture()
    {
        release();
    }
    
    auto StandaloneTexture::loadFromFile(const std::string& path) -> bool
    {
        // First release any existing resources
        release();
    
        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4); // Force RGBA
    
        if (!data) {
            std::cerr << "Failed to load standalone texture: " << path << "\n";
            std::cerr << "Error: " << stbi_failure_reason() << "\n";
            return false;
        }
    
        bool success = loadFromMemory(data, width, height);
    
        // Free the image data as it's now uploaded to GPU
        stbi_image_free(data);
    
        return success;
    }

    auto StandaloneTexture::prepareFromFile(const std::string& path) -> bool
    {
        m_filePath = path;
    
        int width, height, channels;
        m_pixels = stbi_load(path.c_str(), &width, &height, &channels, 4); // Force RGBA
    
        if (!m_pixels) {
            std::cerr << "Failed to prepare standalone texture: " << path << "\n";
            std::cerr << "Error: " << stbi_failure_reason() << "\n";
            return false;
        }
    
        m_width = width;
        m_height = height;
        m_isPrepared = true;
        m_isLoaded = false;
    
        return true;
    }

    auto StandaloneTexture::ensureLoaded() -> bool
    {
        // If already loaded, nothing to do
        if (m_isLoaded)
        {
            return true;
        }
        
        // Check if sokol is valid before attempting to load
        if (!sg_isvalid())
        {
            std::cerr << "ERROR: Cannot load texture - Sokol GFX not initialized\n";
            return false;
        }
    
        // If prepared, create GPU resources now
        if (m_isPrepared && m_pixels)
        {
            bool success = loadFromMemory(m_pixels, m_width, m_height);
            return success;
        }
    
        // If not prepared but we have a file path, load directly
        if (!m_filePath.empty()) {
            return loadFromFile(m_filePath);
        }
    
        std::cerr << "ERROR: Cannot load texture - no prepared pixels or file path\n";
        return false;
    }
    
    auto StandaloneTexture::loadFromMemory(unsigned char* data, glm::i32 width, glm::i32 height) -> bool
    {        
        m_width = width;
        m_height = height;
        
        sg_image_desc img_desc = {};
        img_desc.width = width;
        img_desc.height = height;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.data.subimage[0][0].ptr = data;
        img_desc.data.subimage[0][0].size = (size_t)(width * height * 4);
        
        if (sg_isvalid()) 
        {
            m_image = sg_make_image(&img_desc);
            m_isLoaded = (m_image.id != SG_INVALID_ID);
            
            if (!m_isLoaded) 
            {
                std::cerr << "Failed to create standalone texture image. Size: " << width << "x" << height << "\n";
            }
        } 
        else 
        {
            std::cerr << "ERROR: Attempting to create texture before Sokol GFX is initialized!\n";
            m_isLoaded = false;
        }

        return m_isLoaded;
    }
    
    auto StandaloneTexture::getSize() -> glm::vec2
    {
        return glm::vec2(m_width, m_height);
    }
    
    auto StandaloneTexture::getWidth() -> glm::i32
    {
        return m_width;
    }
    
    auto StandaloneTexture::getHeight() -> glm::i32
    {
        return m_height;
    }
    
    auto StandaloneTexture::getImageHandle() -> sg_image
    {
        if (!m_isLoaded)
        {
            bool loaded = ensureLoaded();
            if (!loaded)
            {
                std::cerr << "WARNING: Failed to load standalone texture before use\n";
            }
        }
        return m_image;
    }
    
    void StandaloneTexture::release()
    {
        if (m_isLoaded && m_image.id != SG_INVALID_ID)
        {
            sg_destroy_image(m_image);
            m_image.id = SG_INVALID_ID;
            m_isLoaded = false;
        }
    
        if (m_pixels)
        {
            free(m_pixels);
            m_pixels = nullptr;
        }
    
        m_isPrepared = false;
    }
}