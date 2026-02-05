//
//  Sprout.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once
#include "Renderer/Font.hpp"
#include "Renderer/StandaloneTexture.hpp"
#include "Renderer/Texture.hpp"
#include "Utility/Color.hpp"

#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "string"
#include <cassert>
#include <cstdint>
#include <memory>
#include <chrono>
#include <array>

#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_glue.h"




namespace Sprout 
{
    
    const int MAX_QUADS = 8192;
    const int MAX_STANDALONE_TEXTURES = 100;
    //const int MAX_VERTS = MAX_QUADS * 4;
        
    
    struct State 
    {
        sg_pipeline pip;
        sg_bindings bind;
        sg_pass_action pass_action;
        sg_buffer quad_vbuf;
        sg_buffer standalone_vbuf;
    };
    
    /*
        * Vertex struct for Sprout.
        * pos (vec4): position of the vertex.
        * color (vec4): color of the vertex.
        * uv (vec2): uv coordinates of the vertex.
        * color_override (vec4): color override of the vertex.
    */
    struct Vertex 
    {
        glm::vec4 pos;
        glm::vec4 color;
        glm::vec2 uv;
        glm::vec4 color_override;
        uint8_t bytes;
        uint8_t padding[3];
        
    };
    
    /*
        * Quad struct for Sprout.
        * vertices (array<Vertex, 4>): the vertices of the quad.
    */
    struct Quad 
    {
        std::array<Vertex, 4> vertices;
    };
    
    /*
        * Atlas struct for Sprout.
        * width (int): width of the atlas.
        * height (int): height of the atlas.
        * img (sg_image): the image of the atlas.
    */
    struct Atlas 
    {
        int width, height;
        sg_image img;
    };
    
    /*
        * DrawFrame struct for Sprout.
        * quads (array<Quad, MAX_QUADS>): the quads to draw.
        * num_quads (int): the number of quads to draw.
        * view_projection (mat4): the view projection matrix.
        * camera_xform (mat4): the camera transform matrix.
    */
    struct DrawFrame
    {
        std::array<Quad, MAX_QUADS> quads;
        int num_quads = 0;
        glm::mat4 view_projection;
        glm::mat4 camera_xform;
        glm::vec4 viewport; // x, y, width, height
        
        std::array<Quad, MAX_STANDALONE_TEXTURES> standalone_quads; // for standalone textures
        sg_image images[MAX_STANDALONE_TEXTURES]; // for standalone textures, indexed by ID
        int num_images = 0; // number of standalone textures
    };
    
    /*
        * Pre-defined pivots for use with Sprout::getPivotOffset.
    */
    enum class Pivot : std::uint8_t
    {
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT,
        CENTER_LEFT,
        CENTER,
        CENTER_RIGHT,
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT
    };
    
    enum class TextJustify : std::uint8_t
    {
        LEFT,
        CENTER,
        RIGHT
    };
    
    glm::vec2 getPivotOffset(Pivot pivot);
    glm::vec2 getAnchorOffset(Pivot pivot);
    
    class Window
    {
        public: 
            Window(int viewportWidth, int viewportHeight, const char* title);
            ~Window();
            static Window* getInstance() { return Instance; }
            DrawFrame draw_frame; 
        
            /*
                * Runs the window, called when everything is initialized
            */
            void Run();
            
            using UpdateFrameCallback = std::function<void(double)>;
            
            /*
                * Sets the update frame callback
                * @param cb The callback to set
            */
            void SetUpdateFrameCallback(UpdateFrameCallback cb);
            
            using EventCallback = std::function<void(const sapp_event*)>;
            
            /*
                * Sets the event callback
                * @param cb The callback to set
            */
            void SetEventCallback(EventCallback cb);
            
            static auto sokol_main() -> sapp_desc;
            
            /*
                * Adds a texture to the window
                * @param tex The texture to add
            */            
            void addTexture(std::shared_ptr<Sprout::Texture> tex);
            
            /*
                * Adds a font to the window
                * @param font The font to add
            */
            void addFont(const std::shared_ptr<Font> font);
        
            /*
                * Draws a sprite to the screen, called from game render code.
                * @param texture The texture to draw
                * @param position The position to draw the sprite
                * @param layer The layer to draw the sprite
                * @param rotation The rotation of the sprite
                * @param frameNumber The frame number of the sprite
                * @param color_override The color override of the sprite
                * @param scale The scale of the sprite
                * @param worldSpace Whether the sprite is in world space or not
            */
            void draw_sprite(
                std::shared_ptr<Sprout::Texture> texture,
                glm::vec2 position, 
                glm::f32 layer,
                glm::f32 rotation = 0.0f,
                glm::i32 frameNumber = 1,
                glm::vec4 color_override = Color::Transparent,
                glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
                Pivot pivot = Pivot::CENTER,
                bool worldSpace = true
            );
            
            /*
                * Draws a rectangle.
                * @param x The x position of the rectangle
                * @param y The y position of the rectangle
                * @param width The width of the rectangle
                * @param height The height of the rectangle
                * @param texture The texture of the rectangle
                * @param color The color of the rectangle
                * @param worldSpace Whether the rectangle is in world space or not
            */
            void draw_rectangle(float x, float y, float width, float height, const std::shared_ptr<Sprout::Texture>& texture, glm::vec4 color = Color::Red, bool worldSpace = true);
            
            /*
                * Renders a text string to the screen.
            */
            void draw_text(const std::string& text, const std::shared_ptr<Font>& font, glm::vec2 position, float depth, glm::vec4 color, float scale = 1.0f, Pivot pivot = Pivot::TOP_LEFT, bool worldSpace = true, TextJustify justify = TextJustify::LEFT);

            /*
                * Draws a standalone texture to the screen.
                * @param texture The stadnalone texture to draw
                * @param position The position to draw the texture
                * @param layer The layer to draw the texture
                * @param rotation The rotation of the texture
                * @param scale The scale of the texture
                * @param pivot The pivot of the texture
                * @param color_override The color override of the texture
            */
            void draw_standalone_texture(
                const std::shared_ptr<Sprout::StandaloneTexture>& texture,
                glm::vec2 position,
                glm::f32 layer,
                glm::f32 rotation = 0.0f,
                glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
                Pivot pivot = Pivot::CENTER,
                glm::vec4 color_override = Color::Transparent
            );

            
            /*
                * Transforms a screen position to a world position.
                * @param screenPos The screen position to transform
                * @return The world position
            */
            static glm::vec2 screenToWorld(glm::vec2 screenPos);
            
            /*
                * Moves the camera by the given delta.
                * @param deltaX The x delta to move the camera
                * @param deltaY The y delta to move the camera
            */
            void translateCamera(glm::f32 deltaX, glm::f32 deltaY);
            
            /*
                * Sets the camera position.
                * @param position The position to set the camera to
            */
            void setCameraPosition(glm::vec2 position);
            
            /*
                * Gets the camera position.
                * @return The camera position
            */
            glm::vec2 getCameraPosition();
            
            /*
                * Updates the viewport based on window dimensions
            */
            void updateViewport();
            
            /*
                * Sets the viewport dimensions
                * @param width The desired viewport width
                * @param height The desired viewport height
            */
            void setViewportSize(int width, int height);
            
            /*
                * Converts window coordinates to viewport coordinates
                * @param windowPos Window coordinates
                * @return Viewport coordinates
            */
            glm::vec2 windowToViewport(glm::vec2 windowPos);

            int getWidth() const { return sapp_width(); }
            int getHeight() const { return sapp_height(); }
        private:
            static Window* Instance;
            
            const char* m_title;

            int m_viewportWidth;
            int m_viewportHeight;
            float m_viewportAspectRatio = 1.0f;

            
            State m_state;
            UpdateFrameCallback m_update_frame_callback;
            EventCallback m_event_callback;
            
            Atlas m_atlas;
            std::vector<std::shared_ptr<Sprout::Texture>> m_textures;
            void bake_atlas();
            
            std::vector<std::shared_ptr<Sprout::Font>> m_fonts;
            void init_fonts();
            std::vector<Atlas> m_fontAtlases;
        
            std::chrono::time_point<std::chrono::system_clock> m_init_time = std::chrono::system_clock::now();
            std::chrono::time_point<std::chrono::system_clock> m_last_frame_time = std::chrono::system_clock::now();
            double m_delta_time = 0.0;
        
            static void init_cb();
            static void frame_cb();
            static void cleanup_cb();
            static void event_cb(const sapp_event* e);
            
            void Init();
            void Frame();
            void Cleanup();
            void Event(const sapp_event* e);
            


            
            void draw_rect_projected(
                glm::mat4 projection, 
                glm::vec2 size, 
                glm::f32 layer,
                glm::vec4 uv,
                glm::vec4 color_override,
                Pivot pivot,
                uint8_t img_tex_id
            );
            
            void draw_quad(
                glm::mat4 projection, 
                std::array<glm::vec4, 4> positions,
                std::array<glm::vec4, 4> colors,
                std::array<glm::vec2, 4> uvs,
                std::array<glm::vec4, 4> color_overrides,
                uint8_t img_tex_id
            );
            
    };
        
} // namespace Sprout