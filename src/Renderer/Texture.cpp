//
//  Texture.cpp
//  SaplingEngine, Sprout Renderer
//


#include "Renderer/Texture.hpp"
#include "Renderer/Sprout.hpp"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


namespace Sprout
{
    
    Texture::~Texture() {}
        
    bool Texture::loadFromFile(const std::string& path, const glm::i32 numFrames) 
    {
        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data)
        {
            return false;
        }
        
        m_pixels = data;
        m_width = width;
        m_height = height;
        
        m_numFrames = numFrames;
        m_frameHeight = m_height;
        m_frameWidth = m_width / m_numFrames;
        
        registerTexture();
        return true;
    }
    
    void Texture::registerTexture()
    {
        Window::getInstance()->addTexture(std::shared_ptr<Texture>(this));
    }
    
    glm::vec2 Texture::getSize()
    {
        return glm::vec2(m_width, m_height);
    }
    
    glm::i32 Texture::getWidth()
    {
        return m_width;
    }
    
    glm::i32 Texture::getHeight()
    {
        return m_height;
    }
    
    glm::vec4 Texture::getAtlasUVs()
    {
        return m_atlas_uvs;
    }
    
    void Texture::setAtlasUVs(glm::vec4 uvs)
    {
        m_atlas_uvs = uvs;
    }
    
    glm::vec2 Texture::getFrameSize()
    {
        return glm::vec2(m_frameWidth, m_frameHeight);
    }
    
    glm::i32 Texture::getNumFrames()
    {
        return m_numFrames;
    }
    
    unsigned char* Texture::getPixels()
    {
        return m_pixels;
    }
    
    bool Texture::loadFromMemory(unsigned char* data, const glm::i32 width, const glm::i32 height, const glm::i32 numFrames)
    {
        m_pixels = new unsigned char[width * height * 4];
        std::memcpy(m_pixels, data, width * height * 4);
        
        m_width = width;
        m_height = height;

        m_numFrames = numFrames;
        m_frameHeight = height;
        m_frameWidth = width;

        return true;
    }
    
    std::vector<std::shared_ptr<Texture>> Texture::loadTileset(const std::string& path, const size_t tileWidth, const size_t tileHeight)
    {
        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
        if (!data)
        {
            throw std::runtime_error("Error loading tileset file: " + path);
        }
        
        std::vector<std::shared_ptr<Texture>> tiles;
        size_t numTiles_x = width / tileWidth;
        size_t numTiles_y = height / tileHeight;
        
        std::vector<unsigned char> tileData(tileWidth * tileHeight * 4);
        
        for (size_t y = 0; y < numTiles_y; y++)
        {
            for (size_t x = 0; x < numTiles_x; x++)
            {
                for (size_t row = 0; row < tileHeight; row++)
                {
                    size_t srcOffset = ((y * tileHeight + row) * width + x * tileWidth) * 4;
                    size_t dstOffset = row * tileWidth * 4;
                    memcpy(&tileData[dstOffset], &data[srcOffset], tileWidth * 4);
                }
                
                std::shared_ptr<Texture> tile = std::make_shared<Texture>();
                tile->loadFromMemory(tileData.data(), tileWidth, tileHeight, 4);
                tiles.push_back(tile);
            }
        }
        
        stbi_image_free(data);
        
        for (auto& tile : tiles)
        {
            tile->registerTexture();
        }
        return tiles;
    }
    
    void Texture::release()
    {
        free(m_pixels);
    }

} // namespace Sprout