//
//  Texture.cpp
//  Sapling Engine, Sprout Renderer
//

#include "Renderer/Texture.hpp"
#include "Renderer/Sprout.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "Core/Logger.hpp"

#include <utility>

namespace Sprout
{

    Texture::~Texture()
    {
        release();
    }

    Texture::Texture(Texture&& other) noexcept
        : m_mode(other.m_mode),
          m_width(other.m_width),
          m_height(other.m_height),
          m_atlas_uvs(other.m_atlas_uvs),
          m_frameWidth(other.m_frameWidth),
          m_frameHeight(other.m_frameHeight),
          m_numFrames(other.m_numFrames),
          m_image(other.m_image),
          m_sampler(other.m_sampler),
          m_samplerCreated(other.m_samplerCreated),
          m_status(other.m_status),
          m_filePath(std::move(other.m_filePath)),
          m_label(std::move(other.m_label)),
          m_pixels(other.m_pixels),
          m_pixelOwnership(other.m_pixelOwnership),
          m_samplerConfig(other.m_samplerConfig),
          m_activeSamplerConfig(other.m_activeSamplerConfig),
          m_retainPixels(other.m_retainPixels),
          m_pixelsPerUnit(other.m_pixelsPerUnit)
    {
        other.m_width = 0;
        other.m_height = 0;
        other.m_frameWidth = 0;
        other.m_frameHeight = 0;
        other.m_numFrames = 1;
        other.m_image = {};
        other.m_sampler = {};
        other.m_samplerCreated = false;
        other.m_status = TextureStatus::Unloaded;
        other.m_pixels = nullptr;
        other.m_pixelOwnership = PixelOwnership::None;
        other.m_pixelsPerUnit = 0.0f;
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
            release();

            m_mode = other.m_mode;
            m_width = other.m_width;
            m_height = other.m_height;
            m_atlas_uvs = other.m_atlas_uvs;
            m_frameWidth = other.m_frameWidth;
            m_frameHeight = other.m_frameHeight;
            m_numFrames = other.m_numFrames;
            m_image = other.m_image;
            m_sampler = other.m_sampler;
            m_samplerCreated = other.m_samplerCreated;
            m_status = other.m_status;
            m_filePath = std::move(other.m_filePath);
            m_label = std::move(other.m_label);
            m_pixels = other.m_pixels;
            m_pixelOwnership = other.m_pixelOwnership;
            m_samplerConfig = other.m_samplerConfig;
            m_activeSamplerConfig = other.m_activeSamplerConfig;
            m_retainPixels = other.m_retainPixels;
            m_pixelsPerUnit = other.m_pixelsPerUnit;

            other.m_width = 0;
            other.m_height = 0;
            other.m_frameWidth = 0;
            other.m_frameHeight = 0;
            other.m_numFrames = 1;
            other.m_image = {};
            other.m_sampler = {};
            other.m_samplerCreated = false;
            other.m_status = TextureStatus::Unloaded;
            other.m_pixels = nullptr;
            other.m_pixelOwnership = PixelOwnership::None;
            other.m_pixelsPerUnit = 0.0f;
        }
        return *this;
    }

    void Texture::setPixelsPerUnit(float pixelsPerUnit)
    {
        m_pixelsPerUnit = pixelsPerUnit > 0.0f ? pixelsPerUnit : 0.0f;
    }

    auto Texture::loadFromFile(const std::string& path, glm::i32 numFrames) -> bool
    {
        release();

        m_filePath = path;

        // atlas mode: no RGBA
        // independent mode: force RGBA
        int reqChannels = (m_mode == TextureMode::Atlas) ? 0 : 4;

        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, reqChannels);

        if (!data)
        {
            Logger::error("Texture: Failed to load '" + path + "': " + stbi_failure_reason());
            m_status = TextureStatus::Error;
            return false;
        }

        m_width = width;
        m_height = height;
        m_numFrames = numFrames;
        updateFrameData();

        if (m_mode == TextureMode::Atlas)
        {
            m_pixels = data;
            m_pixelOwnership = PixelOwnership::Stbi;
            m_status = TextureStatus::Loaded;
            return true;
        }
        else // independent mode
        {
            if (sg_isvalid())
            {
                bool success = uploadToGpu(data, width, height);

                if (success && m_retainPixels)
                {
                    m_pixels = data;
                    m_pixelOwnership = PixelOwnership::Stbi;
                }
                else
                {
                    stbi_image_free(data);
                }

                return success;
            }
            else
            {
                m_pixels = data;
                m_pixelOwnership = PixelOwnership::Stbi;
                m_status = TextureStatus::Prepared;
                return true;
            }
        }
    }

    auto Texture::prepareFromFile(const std::string& path, glm::i32 numFrames) -> bool
    {
        release();

        m_filePath = path;

        int reqChannels = (m_mode == TextureMode::Atlas) ? 0 : 4;

        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, reqChannels);

        if (!data)
        {
            Logger::error("Texture: Failed to prepare '" + path + "': " + stbi_failure_reason());
            m_status = TextureStatus::Error;
            return false;
        }

        m_width = width;
        m_height = height;
        m_numFrames = numFrames;
        updateFrameData();

        m_pixels = data;
        m_pixelOwnership = PixelOwnership::Stbi;
        m_status = TextureStatus::Prepared;

        return true;
    }

    auto Texture::ensureLoaded() -> bool
    {
        if (m_mode == TextureMode::Atlas)
        {
            return m_status == TextureStatus::Loaded || m_status == TextureStatus::Prepared;
        }

        if (m_status == TextureStatus::Loaded)
        {
            return true;
        }

        if (!sg_isvalid())
        {
            Logger::error("Texture: Cannot upload — Sokol GFX not initialized");
            return false;
        }

        if (m_status == TextureStatus::Prepared && m_pixels)
        {
            bool success = uploadToGpu(m_pixels, m_width, m_height);

            if (success && !m_retainPixels)
            {
                freePixels();
            }

            return success;
        }

        if (!m_filePath.empty())
        {
            return loadFromFile(m_filePath, m_numFrames);
        }

        Logger::error("Texture: Cannot load — no prepared pixels or file path");
        m_status = TextureStatus::Error;
        return false;
    }

    auto Texture::loadFromMemory(unsigned char* data, glm::i32 width, glm::i32 height, glm::i32 numFrames) -> bool
    {
        release();

        if (!data || width <= 0 || height <= 0)
        {
            Logger::error("Texture: Invalid data passed to loadFromMemory (data=" + std::string(data ? "valid" : "null") + ", size=" + std::to_string(width) + "x" + std::to_string(height) + ")");
            m_status = TextureStatus::Error;
            return false;
        }

        m_width = width;
        m_height = height;
        m_numFrames = numFrames;
        updateFrameData();

        if (m_mode == TextureMode::Atlas)
        {
            size_t dataSize = static_cast<size_t>(width) * height * 4;
            m_pixels = new unsigned char[dataSize];
            std::memcpy(m_pixels, data, dataSize);
            m_pixelOwnership = PixelOwnership::Copied;
            m_status = TextureStatus::Loaded;
            return true;
        }
        else
        {
            if (sg_isvalid())
            {
                bool success = uploadToGpu(data, width, height);

                if (success && m_retainPixels)
                {
                    size_t dataSize = static_cast<size_t>(width) * height * 4;
                    m_pixels = new unsigned char[dataSize];
                    std::memcpy(m_pixels, data, dataSize);
                    m_pixelOwnership = PixelOwnership::Copied;
                }

                return success;
            }
            else
            {
                size_t dataSize = static_cast<size_t>(width) * height * 4;
                m_pixels = new unsigned char[dataSize];
                std::memcpy(m_pixels, data, dataSize);
                m_pixelOwnership = PixelOwnership::Copied;
                m_status = TextureStatus::Prepared;
                return true;
            }
        }
    }

    auto Texture::reload() -> bool
    {
        if (m_filePath.empty())
        {
            Logger::error("Texture: Cannot reload — no file path set");
            return false;
        }

        glm::i32 savedFrames = m_numFrames;
        release();

        if (m_mode == TextureMode::Independent && sg_isvalid())
        {
            return loadFromFile(m_filePath, savedFrames);
        }
        else
        {
            return prepareFromFile(m_filePath, savedFrames);
        }
    }

    void Texture::registerTexture()
    {
        if (m_mode != TextureMode::Atlas)
        {
            Logger::warn("Texture: registerTexture() called on non-Atlas texture — ignoring");
            return;
        }

        Window::getInstance()->addTexture(shared_from_this());
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

                auto tile = std::make_shared<Texture>();
                // tiles are always in atlas mode (default)
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


    auto Texture::getPixels() -> unsigned char*
    {
        return m_pixels;
    }

    auto Texture::getSize() const -> glm::vec2
    {
        return glm::vec2(m_width, m_height);
    }

    auto Texture::getWidth() const -> glm::i32
    {
        return m_width;
    }

    auto Texture::getHeight() const -> glm::i32
    {
        return m_height;
    }

    auto Texture::isReady() const -> bool
    {
        if (m_mode == TextureMode::Atlas)
        {
            return m_status == TextureStatus::Loaded || m_status == TextureStatus::Prepared;
        }
        return m_status == TextureStatus::Loaded;
    }

    auto Texture::getAtlasUVs() const -> glm::vec4
    {
        return m_atlas_uvs;
    }

    void Texture::setAtlasUVs(glm::vec4 uvs)
    {
        m_atlas_uvs = uvs;
    }

    auto Texture::getFrameSize() const -> glm::vec2
    {
        return glm::vec2(m_frameWidth, m_frameHeight);
    }

    auto Texture::getNumFrames() const -> glm::i32
    {
        return m_numFrames;
    }

    auto Texture::getImageHandle() -> sg_image
    {
        if (m_mode != TextureMode::Independent)
        {
            Logger::error("Texture: getImageHandle() called on Atlas-mode texture" + (m_filePath.empty() ? "" : " (file: " + m_filePath + ")"));
            return m_image;
        }

        if (m_status != TextureStatus::Loaded)
        {
            if (!ensureLoaded())
            {
                std::string msg = "Texture: Failed to ensure texture loaded before use";
                if (!m_filePath.empty()) msg += " (file: " + m_filePath + ")";
                if (!m_label.empty()) msg += " [" + m_label + "]";
                Logger::error(msg);
            }
        }
        return m_image;
    }


    void Texture::setSamplerConfig(const SamplerConfig& config)
    {
        m_samplerConfig = config;
    }

    auto Texture::getSampler() -> sg_sampler
    {
        bool needsCreate = !m_samplerCreated || (m_samplerConfig != m_activeSamplerConfig);

        if (needsCreate)
        {
            if (!sg_isvalid())
            {
                Logger::error("Texture: Cannot create sampler — Sokol GFX not initialized");
                return m_sampler;
            }

            if (m_samplerCreated && m_sampler.id != SG_INVALID_ID)
            {
                sg_destroy_sampler(m_sampler);
            }

            sg_sampler_desc desc = {};
            desc.min_filter = m_samplerConfig.minFilter;
            desc.mag_filter = m_samplerConfig.magFilter;
            desc.mipmap_filter = m_samplerConfig.mipmapFilter;
            desc.wrap_u = m_samplerConfig.wrapU;
            desc.wrap_v = m_samplerConfig.wrapV;

            std::string samplerLabel;
            if (!m_label.empty())
            {
                samplerLabel = m_label + "-sampler";
                desc.label = samplerLabel.c_str();
            }

            m_sampler = sg_make_sampler(&desc);
            m_activeSamplerConfig = m_samplerConfig;
            m_samplerCreated = true;
        }

        return m_sampler;
    }


    void Texture::release()
    {
        if (m_mode == TextureMode::Independent)
        {
            if (m_image.id != SG_INVALID_ID && m_status == TextureStatus::Loaded)
            {
                if (sg_isvalid())
                {
                    sg_destroy_image(m_image);
                }
                m_image = {};
            }
        }

        if (m_samplerCreated && m_sampler.id != SG_INVALID_ID)
        {
            if (sg_isvalid())
            {
                sg_destroy_sampler(m_sampler);
            }
            m_sampler = {};
            m_samplerCreated = false;
        }

        freePixels();

        m_width = 0;
        m_height = 0;
        m_frameWidth = 0;
        m_frameHeight = 0;
        m_numFrames = 1;
        m_atlas_uvs = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        m_status = TextureStatus::Unloaded;
    }

    void Texture::releasePixels()
    {
        freePixels();
    }


    auto Texture::uploadToGpu(const unsigned char* data, glm::i32 width, glm::i32 height) -> bool
    {
        if (!data || width <= 0 || height <= 0)
        {
            Logger::error("Texture: Cannot upload — invalid data or dimensions");
            m_status = TextureStatus::Error;
            return false;
        }

        if (!sg_isvalid())
        {
            Logger::error("Texture: Cannot upload — Sokol GFX not initialized");
            m_status = TextureStatus::Error;
            return false;
        }

        sg_image_desc img_desc = {};
        img_desc.width = width;
        img_desc.height = height;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.data.subimage[0][0].ptr = data;
        img_desc.data.subimage[0][0].size = static_cast<size_t>(width) * height * 4;

        std::string derivedLabel;
        if (!m_label.empty())
        {
            img_desc.label = m_label.c_str();
        }
        else if (!m_filePath.empty())
        {
            size_t lastSlash = m_filePath.find_last_of("/\\");
            derivedLabel = (lastSlash != std::string::npos)
                ? m_filePath.substr(lastSlash + 1)
                : m_filePath;
            img_desc.label = derivedLabel.c_str();
        }

        m_image = sg_make_image(&img_desc);

        if (m_image.id == SG_INVALID_ID)
        {
            Logger::error("Texture: sg_make_image failed for " + std::to_string(width) + "x" + std::to_string(height) + " texture" + (m_filePath.empty() ? "" : " (file: " + m_filePath + ")"));
            m_status = TextureStatus::Error;
            return false;
        }

        m_status = TextureStatus::Loaded;
        return true;
    }

    void Texture::freePixels()
    {
        if (!m_pixels) return;

        switch (m_pixelOwnership)
        {
            case PixelOwnership::Stbi:
                free(m_pixels);
                break;

            case PixelOwnership::Copied:
                delete[] m_pixels;
                break;

            case PixelOwnership::None:
                break;
        }

        m_pixels = nullptr;
        m_pixelOwnership = PixelOwnership::None;
    }

    void Texture::updateFrameData()
    {
        m_frameHeight = m_height;
        m_frameWidth = (m_numFrames > 0) ? (m_width / m_numFrames) : m_width;
    }

} // namespace Sprout
