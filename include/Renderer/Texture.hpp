//
//  Texture.hpp
//  Sapling Engine, Sprout Renderer
//


#pragma once

#include <glm/glm.hpp>
#include "sokol/sokol_gfx.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace Sprout
{

    // atlas is packed into shared image, independent owns its own gpu image
    enum class TextureMode : uint8_t
    {
        Atlas,
        Independent
    };

    enum class TextureStatus : uint8_t
    {
        Unloaded,
        Prepared,
        Loaded,
        Error
    };

    struct SamplerConfig
    {
        sg_filter minFilter = SG_FILTER_LINEAR;
        sg_filter magFilter = SG_FILTER_LINEAR;
        sg_filter mipmapFilter = SG_FILTER_LINEAR;
        sg_wrap wrapU = SG_WRAP_REPEAT;
        sg_wrap wrapV = SG_WRAP_REPEAT;

        static SamplerConfig nearest()
        {
            SamplerConfig cfg;
            cfg.minFilter = SG_FILTER_NEAREST;
            cfg.magFilter = SG_FILTER_NEAREST;
            cfg.mipmapFilter = SG_FILTER_NEAREST;
            cfg.wrapU = SG_WRAP_CLAMP_TO_EDGE;
            cfg.wrapV = SG_WRAP_CLAMP_TO_EDGE;
            return cfg;
        }

        static SamplerConfig linearClamp()
        {
            SamplerConfig cfg;
            cfg.wrapU = SG_WRAP_CLAMP_TO_EDGE;
            cfg.wrapV = SG_WRAP_CLAMP_TO_EDGE;
            return cfg;
        }

        static SamplerConfig linearRepeat()
        {
            return SamplerConfig{};
        }

        bool operator==(const SamplerConfig& other) const
        {
            return minFilter == other.minFilter
                && magFilter == other.magFilter
                && mipmapFilter == other.mipmapFilter
                && wrapU == other.wrapU
                && wrapV == other.wrapV;
        }

        bool operator!=(const SamplerConfig& other) const
        {
            return !(*this == other);
        }
    };

    enum class PixelOwnership : uint8_t
    {
        None,
        Stbi,
        Copied
    };

    class Window;

    class Texture : public std::enable_shared_from_this<Texture>
    {
    public:
        Texture() = default;
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;


        /*
         * Loads a texture from a file.
         *
         * Atlas mode: loads pixels to CPU for atlas packing. Call registerTexture()
         * separately to add it to the atlas (or let AssetManager handle it).
         *
         * Independent mode: loads pixels to CPU, then uploads to GPU if Sokol is
         * ready. CPU pixels freed after upload unless retainPixels is set.
         *
         * @param path The path to the texture file
         * @param numFrames The number of frames in the texture (if animated)
         * @return True if the texture was loaded successfully
         */
        auto loadFromFile(const std::string& path, glm::i32 numFrames = 1) -> bool;

        /*
         * Prepares a texture from a file but delays GPU upload until
         * ensureLoaded() is called. Useful for loading before Sokol init.
         * Only meaningful for Independent mode.
         * @param path The path to the texture file
         * @param numFrames The number of frames in the texture (if animated)
         * @return True if the file was read successfully
         */
        auto prepareFromFile(const std::string& path, glm::i32 numFrames = 1) -> bool;

        /*
         * Ensures the texture is uploaded to GPU memory.
         * Independent mode: uploads to GPU if not already done.
         * Atlas mode: no-op (atlas baking handles GPU upload).
         * Safe to call multiple times — returns immediately if already loaded.
         * @return True if the texture is ready for use
         */
        auto ensureLoaded() -> bool;

        /*
         * Loads a texture from pixel data in memory.
         * @param data The RGBA pixel data
         * @param width The width in pixels
         * @param height The height in pixels
         * @param numFrames The number of frames in the texture (if animated)
         * @return True if the texture was loaded successfully
         */
        auto loadFromMemory(unsigned char* data, glm::i32 width, glm::i32 height, glm::i32 numFrames = 1) -> bool;

        /*
         * Reloads the texture from its original file path.
         * Only works if the texture was loaded/prepared from a file.
         * @return True if reload succeeded
         */
        auto reload() -> bool;

        /*
         * Registers the texture with the renderer (adds it to the atlas).
         * Only meaningful for Atlas mode textures.
         */
        void registerTexture();

        /*
         * Loads a tileset from a file. Each tile becomes an Atlas-mode Texture.
         * @param path The path to the tileset file
         * @param tileWidth The width of each tile in the tileset
         * @param tileHeight The height of each tile in the tileset
         * @return A vector of textures representing the tiles in the tileset
         */
        static auto loadTileset(const std::string& path, const size_t tileWidth, const size_t tileHeight) -> std::vector<std::shared_ptr<Texture>>;


        /*
         * Gets the raw CPU pixel data.
         * Atlas mode: always available (needed for atlas baking).
         * Independent mode: may be nullptr after GPU upload.
         */
        auto getPixels() -> unsigned char*;

        auto getSize() const -> glm::vec2;
        auto getWidth() const -> glm::i32;
        auto getHeight() const -> glm::i32;

        auto getStatus() const -> TextureStatus { return m_status; }
        auto isReady() const -> bool;
        auto getMode() const -> TextureMode { return m_mode; }
        auto getFilePath() const -> const std::string& { return m_filePath; }
        auto hasPixels() const -> bool { return m_pixels != nullptr; }
        void setPixelsPerUnit(float pixelsPerUnit);
        auto getPixelsPerUnit() const -> float { return m_pixelsPerUnit; }
        auto hasPixelsPerUnitOverride() const -> bool { return m_pixelsPerUnit > 0.0f; }


        /*
         * Gets the UV coordinates of the texture in the atlas.
         * Only meaningful for Atlas mode.
         */
        auto getAtlasUVs() const -> glm::vec4;

        /*
         * Sets the UV coordinates of the texture in the atlas.
         * Called by the atlas baker (Window::bake_atlas).
         */
        void setAtlasUVs(glm::vec4 uvs);

        /*
         * Gets the size of a single frame in the texture.
         * For non-animated textures, this equals getSize().
         */
        auto getFrameSize() const -> glm::vec2;

        /*
         * Gets the number of frames in the texture.
         */
        auto getNumFrames() const -> glm::i32;

        /*
         * Gets the Sokol image handle. Automatically calls ensureLoaded()
         * if the texture is prepared but not yet uploaded.
         * Only meaningful for Independent mode.
         * @return The sg_image handle (may be invalid if load failed or Atlas mode)
         */
        auto getImageHandle() -> sg_image;

        /*
         * Sets the texture mode. Must be called BEFORE loading.
         * @param mode The texture mode
         */
        void setMode(TextureMode mode) { m_mode = mode; }

        /*
         * Sets the sampler configuration (filtering, wrapping).
         * Only used by Independent mode textures.
         * If the sampler has already been created, it will be recreated
         * on the next getSampler() call.
         */
        void setSamplerConfig(const SamplerConfig& config);

        auto getSamplerConfig() const -> const SamplerConfig& { return m_samplerConfig; }

        /*
         * Gets or lazily creates a Sokol sampler matching the current config.
         * Only meaningful for Independent mode.
         * @return The sg_sampler handle
         */
        auto getSampler() -> sg_sampler;

        /*
         * When true, CPU pixel data is kept in memory after GPU upload.
         * Default is false for Independent mode (pixels freed after upload).
         * Atlas mode always retains pixels (needed for atlas baking).
         */
        void setRetainPixels(bool retain) { m_retainPixels = retain; }
        auto getRetainPixels() const -> bool { return m_retainPixels; }

        /*
         * Sets a debug label for this texture (shown in Sokol GPU debuggers).
         * Must be set before ensureLoaded() / loadFromFile() to take effect.
         */
        void setLabel(const std::string& label) { m_label = label; }
        auto getLabel() const -> const std::string& { return m_label; }

        /*
         * Releases all GPU and CPU resources. Resets to Unloaded state.
         */
        void release();

        /*
         * Releases only the CPU pixel data, keeping the GPU image alive.
         * Only meaningful for Independent mode.
         */
        void releasePixels();

    private:
        TextureMode m_mode = TextureMode::Atlas;

        glm::i32 m_width = 0;
        glm::i32 m_height = 0;

        // atlas data
        glm::vec4 m_atlas_uvs = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        glm::i32 m_frameWidth = 0;
        glm::i32 m_frameHeight = 0;
        glm::i32 m_numFrames = 1;

        // independent mode gpu resources
        sg_image m_image = {};
        sg_sampler m_sampler = {};
        bool m_samplerCreated = false;

        TextureStatus m_status = TextureStatus::Unloaded;
        std::string m_filePath;
        std::string m_label;

        unsigned char* m_pixels = nullptr;
        PixelOwnership m_pixelOwnership = PixelOwnership::None;

        SamplerConfig m_samplerConfig;
        SamplerConfig m_activeSamplerConfig;
        bool m_retainPixels = false;
        float m_pixelsPerUnit = 0.0f;

        auto uploadToGpu(const unsigned char* data, glm::i32 width, glm::i32 height) -> bool;
        void freePixels();
        void updateFrameData();
    };

} // namespace Sprout
