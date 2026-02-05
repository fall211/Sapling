//
//  Window.cpp
//  SaplingEngine, Sprout Renderer
//

//  This file contains all the window functionality that is 
//  related to windowing but not part of sokol implementation.
//  For the sokol window setup, see Sprout.mm

#include "Renderer/Font.hpp"
#include "Renderer/Sprout.hpp"
#include "Renderer/quad.h"
#include "Utility/Debug.hpp"
#include "Utility/Color.hpp"

#include "glm/fwd.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <cstdint>
#include <memory>
#include <string>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

namespace Sprout
{

    void Window::bake_atlas()
    {
        const int PADDING = 2;
            m_atlas.width = 1024;
            m_atlas.height = 1024;
            
            unsigned char* atlas_data = (unsigned char*)malloc(m_atlas.width * m_atlas.height * 4);
            memset((void*)atlas_data, 0, m_atlas.width * m_atlas.height * 4);
            
            stbrp_context context;
            stbrp_node* nodes = (stbrp_node*)malloc(sizeof(stbrp_node) * m_atlas.width);
            stbrp_init_target(&context, m_atlas.width, m_atlas.height, nodes, m_atlas.width);
            
            for (auto& tex : m_textures) {
                stbrp_rect rect;
                rect.id = 0;
                rect.w = tex->getWidth() + (PADDING * 2);
                rect.h = tex->getHeight() + (PADDING * 2);
                
                if (!stbrp_pack_rects(&context, &rect, 1)) {
                    throw std::runtime_error("Failed to pack rects");
                    continue;
                }
                
                // copy texture data
                for (int y = 0; y < tex->getHeight(); y++) {
                    for (int x = 0; x < tex->getWidth(); x++) {
                        int src_index = (y * tex->getWidth() + x) * 4;
                        int dst_index = ((rect.y + PADDING + y) * m_atlas.width + (rect.x + PADDING + x)) * 4;
                        
                        atlas_data[dst_index + 0] = tex->getPixels()[src_index + 0];
                        atlas_data[dst_index + 1] = tex->getPixels()[src_index + 1];
                        atlas_data[dst_index + 2] = tex->getPixels()[src_index + 2];
                        atlas_data[dst_index + 3] = tex->getPixels()[src_index + 3];
                    }
                }
                
                // extend edges into padding
                int base_x = rect.x + PADDING;
                int base_y = rect.y + PADDING;
                int width = tex->getWidth();
                int height = tex->getHeight();
                
                // horizontal edges
                for (int x = 0; x < width; x++) {
                    // T
                    for (int p = 1; p <= PADDING; p++) {
                        int src_index = ((base_y) * m_atlas.width + (base_x + x)) * 4;
                        int dst_index = ((base_y - p) * m_atlas.width + (base_x + x)) * 4;
                        memcpy(&atlas_data[dst_index], &atlas_data[src_index], 4);
                    }
                    
                    // B
                    for (int p = 1; p <= PADDING; p++) {
                        int src_index = ((base_y + height - 1) * m_atlas.width + (base_x + x)) * 4;
                        int dst_index = ((base_y + height - 1 + p) * m_atlas.width + (base_x + x)) * 4;
                        memcpy(&atlas_data[dst_index], &atlas_data[src_index], 4);
                    }
                }
                
                // vertical edges
                for (int y = 0; y < height; y++) {
                    // L
                    for (int p = 1; p <= PADDING; p++) {
                        int src_index = ((base_y + y) * m_atlas.width + base_x) * 4;
                        int dst_index = ((base_y + y) * m_atlas.width + (base_x - p)) * 4;
                        memcpy(&atlas_data[dst_index], &atlas_data[src_index], 4);
                    }
                    
                    // R
                    for (int p = 1; p <= PADDING; p++) {
                        int src_index = ((base_y + y) * m_atlas.width + (base_x + width - 1)) * 4;
                        int dst_index = ((base_y + y) * m_atlas.width + (base_x + width - 1 + p)) * 4;
                        memcpy(&atlas_data[dst_index], &atlas_data[src_index], 4);
                    }
                }
                
                // corners
                for (int py = 1; py <= PADDING; py++) {
                    for (int px = 1; px <= PADDING; px++) {
                        // TL
                        memcpy(
                            &atlas_data[((base_y - py) * m_atlas.width + (base_x - px)) * 4],
                            &atlas_data[(base_y * m_atlas.width + base_x) * 4],
                            4
                        );
                        
                        // TR
                        memcpy(
                            &atlas_data[((base_y - py) * m_atlas.width + (base_x + width - 1 + px)) * 4],
                            &atlas_data[(base_y * m_atlas.width + (base_x + width - 1)) * 4],
                            4
                        );
                        
                        // BL
                        memcpy(
                            &atlas_data[((base_y + height - 1 + py) * m_atlas.width + (base_x - px)) * 4],
                            &atlas_data[((base_y + height - 1) * m_atlas.width + base_x) * 4],
                            4
                        );
                        
                        // BR
                        memcpy(
                            &atlas_data[((base_y + height - 1 + py) * m_atlas.width + (base_x + width - 1 + px)) * 4],
                            &atlas_data[((base_y + height - 1) * m_atlas.width + (base_x + width - 1)) * 4],
                            4
                        );
                    }
                }
                
                glm::vec4 uv = glm::vec4(
                    (float)(rect.x + PADDING) / m_atlas.width,
                    (float)(rect.y + PADDING) / m_atlas.height,
                    (float)(rect.x + PADDING + tex->getWidth()) / m_atlas.width,
                    (float)(rect.y + PADDING + tex->getHeight()) / m_atlas.height
                );
                tex->setAtlasUVs(uv);
            }
            
            sg_image_desc img_desc = {};
            img_desc.width = m_atlas.width;
            img_desc.height = m_atlas.height;
            img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            img_desc.data.subimage[0][0].ptr = atlas_data;
            img_desc.data.subimage[0][0].size = (size_t)m_atlas.width * m_atlas.height * 4;
 
            
            m_atlas.img = sg_make_image(&img_desc);
            
            #ifdef DEBUG
            stbi_write_png("atlas.png", m_atlas.width, m_atlas.height, 4, atlas_data, m_atlas.width * 4);
            #endif
            
            free(atlas_data);
            free(nodes);
    }
    
    void Window::addTexture(const std::shared_ptr<Texture> tex)
    {
        m_textures.push_back(tex);
    }
    
    void Window::init_fonts()
    {
        int atlas_width = 512;
        int atlas_height = 512;

        for (size_t i = 0; i < m_fonts.size(); i++)
        {
            float font_size = m_fonts[i]->size;
            unsigned char* atlas_data = new unsigned char[atlas_width * atlas_height];
            memset(atlas_data, 0, atlas_width * atlas_height);

            stbtt_BakeFontBitmap(
                m_fonts[i]->data,           // unsigned char* font data
                0,                          // font offset
                font_size,                  // pixel height
                atlas_data,                 // output bitmap
                atlas_width,                // output bitmap width
                atlas_height,               // output bitmap height
                32,                         // first character (space)
                96,                         // number of characters
                m_fonts[i]->bakedChars      // output character data
            );

            std::vector<unsigned char> rgba_data(atlas_width * atlas_height * 4, 0);
            for (int j = 0; j < atlas_width * atlas_height; j++) {
                unsigned char alpha = atlas_data[j];
                rgba_data[j * 4 + 0] = 255;  // R
                rgba_data[j * 4 + 1] = 255;  // G
                rgba_data[j * 4 + 2] = 255;  // B
                rgba_data[j * 4 + 3] = alpha;  // A
            }

            sg_image_desc img_desc = {};
            img_desc.width = atlas_width;
            img_desc.height = atlas_height;
            img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            img_desc.data.subimage[0][0].ptr = rgba_data.data();
            img_desc.data.subimage[0][0].size = rgba_data.size();

            Atlas fontAtlas;
            fontAtlas.img = sg_make_image(&img_desc);
            fontAtlas.width = atlas_width;
            fontAtlas.height = atlas_height;
            
            m_fontAtlases.push_back(fontAtlas);
            
            m_fonts[i]->fontId = i;
            
            #ifdef DEBUG
            std::string filename = "fontAtlas" + std::to_string(i) + ".png";
            stbi_write_png(filename.c_str(), atlas_width, atlas_height, 1, atlas_data, atlas_width);
            #endif
            
            delete[] atlas_data;
        }
    }
    
    void Window::addFont(const std::shared_ptr<Font> font)
    {
        m_fonts.push_back(font);
    }
    
    void Window::SetUpdateFrameCallback(UpdateFrameCallback callback)
    {
        m_update_frame_callback = std::move(callback);
        if (m_update_frame_callback) {
        }
    }
    
    void Window::SetEventCallback(EventCallback callback)
    {
        m_event_callback = std::move(callback);
    }
    
    
    //  RENDERING
    
    void Window::draw_sprite(
        const std::shared_ptr<Sprout::Texture> texture,
        const glm::vec2 position, 
        const glm::f32 layer,
        const glm::f32 rotation,
        const glm::i32 frameNum,
        const glm::vec4 color_override,
        const glm::vec3 scale,
        const Pivot pivot,
        const bool worldSpace)
    {   
        glm::mat4 xform0 = glm::mat4(1.0f);
        

        // translate
        if (worldSpace)
        {
            xform0 = glm::translate(xform0, glm::vec3(position, 0.0f));     
        }
        else // ui elements need to use pivot for an anchor as well as a pivot
        {
            glm::vec2 anchor_offset = getAnchorOffset(pivot);
            glm::vec2 pos = glm::vec2(position.x * anchor_offset.x * -1, position.y * anchor_offset.y * -1);
            if (anchor_offset.x == 0)       pos.x = position.x;
            if (anchor_offset.y == 0)       pos.y = -position.y;
            
            anchor_offset.x = anchor_offset.x * m_viewportWidth + pos.x;
            anchor_offset.y = anchor_offset.y * m_viewportHeight + pos.y;
            
            xform0 = glm::translate(xform0, glm::vec3(anchor_offset, 0.0f));       
        }
        
        // scale
        xform0 = glm::scale(xform0, scale);
        
        // rotate
        xform0 = glm::rotate(xform0, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        
        glm::vec2 size = texture->getSize();                
        glm::vec2 frame_size = texture->getFrameSize();
        glm::vec4 uv = texture->getAtlasUVs();
        
        if (frame_size != size)
        {
            glm::i32 numFrames = texture->getNumFrames();
            
            glm::vec4 offset_uv = glm::vec4(
                uv.x + ((uv.z - uv.x) / (float)numFrames) * (float)frameNum,
                uv.y,
                uv.x + ((uv.z - uv.x) / (float)numFrames) * ((float)frameNum + 1),
                uv.w
            );
            uv = offset_uv;
        }
        
        // draw
        if (worldSpace)
        {
            draw_rect_projected(draw_frame.view_projection * draw_frame.camera_xform * xform0, frame_size, layer, uv, color_override, pivot, 0);
        }
        else // ui elements are screen space
        {
            draw_rect_projected(draw_frame.view_projection * xform0, frame_size, layer, uv, color_override, pivot, 0);
        }
    }
    
    void Window::draw_rectangle(
        float x, 
        float y, 
        float width, 
        float height, 
        const std::shared_ptr<Sprout::Texture>& texture, 
        glm::vec4 color,
        bool worldSpace)
    {
        // draw rectangle
        auto xform = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
        glm::vec4 uv = texture->getAtlasUVs();

        if (worldSpace)
        {
            draw_rect_projected(draw_frame.view_projection * draw_frame.camera_xform * xform, glm::vec2(width, height), 1.0f, uv, color, Pivot::CENTER, 0);
        }
        else
        {
            draw_rect_projected(draw_frame.view_projection * xform, glm::vec2(width, height), 1.0f, uv, color, Pivot::CENTER, 0);
        }
    }
    
    
    void Window::draw_rect_projected(
        glm::mat4 projection,
        glm::vec2 size,
        glm::f32 layer,
        glm::vec4 uv,
        glm::vec4 color_override,
        Pivot pivot,
        uint8_t img_tex_id)
    {
        glm::vec2 pivot_offset = getPivotOffset(pivot) * size;


        glm::vec4 bottom_left = glm::vec4(0.0f - pivot_offset.x, 0.0f - pivot_offset.y, layer, 1.0f);
        glm::vec4 bottom_right = glm::vec4(size.x - pivot_offset.x, 0.0f - pivot_offset.y, layer, 1.0f);
        glm::vec4 top_left = glm::vec4(0.0f - pivot_offset.x, size.y - pivot_offset.y, layer, 1.0f);
        glm::vec4 top_right = glm::vec4(size.x - pivot_offset.x, size.y - pivot_offset.y, layer, 1.0f);
        
        std::array<glm::vec4, 4> positions = {bottom_left, top_left, top_right, bottom_right};
        glm::vec4 color = Color::White;;
        std::array<glm::vec4, 4> colors = {color, color, color, color};
        std::array<glm::vec2, 4> uvs = {
            glm::vec2(uv.x, uv.y),
            glm::vec2(uv.x, uv.w),
            glm::vec2(uv.z, uv.w),
            glm::vec2(uv.z, uv.y)
        };
        std::array<glm::vec4, 4> color_overrides = {color_override, color_override, color_override, color_override};
        
        draw_quad(projection, positions, colors, uvs, color_overrides, img_tex_id) ; 
    }
    
    
    void Window::draw_quad(
        glm::mat4 projection, 
        std::array<glm::vec4, 4> positions,
        std::array<glm::vec4, 4> colors,
        std::array<glm::vec2, 4> uvs,
        std::array<glm::vec4, 4> color_overrides,
        uint8_t img_tex_id)
    {
        if (draw_frame.num_quads > MAX_QUADS)
        {
            //throw std::runtime_error("Too many quads");
            return;
        }
        
        for (int i = 0; i < 4; i++)
        {
            Vertex& vertex = draw_frame.quads[draw_frame.num_quads].vertices[i];
            
            vertex.pos = projection * positions[i];
            vertex.color = colors[i];
            vertex.uv = uvs[i];
            vertex.color_override = color_overrides[i];
            vertex.bytes = img_tex_id;
        }
        
        draw_frame.num_quads++;
    }
    
    void Window::draw_standalone_texture(
        const std::shared_ptr<Sprout::StandaloneTexture>& texture,
        glm::vec2 position,
        glm::f32 depth,
        glm::f32 rotation,
        glm::vec3 scale,
        Pivot pivot,
        glm::vec4 color_override)
    {
        if (draw_frame.num_images >= MAX_STANDALONE_TEXTURES) {
            return; // no more space for standalone textures
        }
        
        glm::mat4 xform0 = glm::mat4(1.0f);
        

        // translate
        xform0 = glm::translate(xform0, glm::vec3(position, 0.0f));
        
        // scale
        xform0 = glm::scale(xform0, scale);
        
        // rotate
        xform0 = glm::rotate(xform0, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        
        // draw
        
        glm::vec2 size = texture->getSize();
        
        glm::vec2 pivot_offset = getPivotOffset(pivot) * size;


        glm::vec4 bottom_left = glm::vec4(0.0f - pivot_offset.x, 0.0f - pivot_offset.y, depth, 1.0f);
        glm::vec4 bottom_right = glm::vec4(size.x - pivot_offset.x, 0.0f - pivot_offset.y, depth, 1.0f);
        glm::vec4 top_left = glm::vec4(0.0f - pivot_offset.x, size.y - pivot_offset.y, depth, 1.0f);
        glm::vec4 top_right = glm::vec4(size.x - pivot_offset.x, size.y - pivot_offset.y, depth, 1.0f);
        
        std::array<glm::vec4, 4> positions = {bottom_left, top_left, top_right, bottom_right};
        glm::vec4 color = Color::White;;
        std::array<glm::vec4, 4> colors = {color, color, color, color};
        std::array<glm::vec2, 4> uvs = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f)
        };
        std::array<glm::vec4, 4> color_overrides = {color_override, color_override, color_override, color_override};
        
        glm::mat4 projection = draw_frame.view_projection * draw_frame.camera_xform * xform0;
        
        for (int i = 0; i < 4; i++)
        {
            Vertex& vertex = draw_frame.standalone_quads[draw_frame.num_images].vertices[i];
            
            vertex.pos = projection * positions[i];
            vertex.color = colors[i];
            vertex.uv = uvs[i];
            vertex.color_override = color_overrides[i];
            vertex.bytes = 0;
        }
        draw_frame.images[draw_frame.num_images] = texture->getImageHandle();
        draw_frame.num_images++;
    }
    
    glm::vec2 Window::screenToWorld(glm::vec2 screen_pos) 
    {
        glm::vec2 viewport_pos = Instance->windowToViewport(screen_pos);
        
        if (viewport_pos.x < 0 || viewport_pos.y < 0) {
            return glm::vec2(-1.0f, -1.0f);
        }
    
        glm::mat4 inv_proj = glm::inverse(Window::getInstance()->draw_frame.view_projection * Window::getInstance()->draw_frame.camera_xform);
 
        glm::vec4 world_pos = inv_proj * glm::vec4(viewport_pos.x, viewport_pos.y, 0.0f, 1.0f);
        
        return glm::vec2(world_pos.x, world_pos.y);
    }
    
        void Window::updateViewport()
        {
            float window_aspect = static_cast<float>(sapp_width()) / static_cast<float>(sapp_height());
    
            int viewport_x, viewport_y, viewport_width, viewport_height;
    
            if (window_aspect > m_viewportAspectRatio) {
                // window is wider than the target aspect - letterboxing on sides
                viewport_height = sapp_height();
                viewport_width = static_cast<int>(sapp_height() * m_viewportAspectRatio);
                viewport_y = 0;
                viewport_x = (sapp_width() - viewport_width) / 2;
            } else {
                // window is taller than the target aspect - letterboxing on top/bottom
                viewport_width = sapp_width();
                viewport_height = static_cast<int>(sapp_width() / m_viewportAspectRatio);
                viewport_x = 0;
                viewport_y = (sapp_height() - viewport_height) / 2;
            }
    
            draw_frame.viewport = glm::vec4(viewport_x, viewport_y, viewport_width, viewport_height);
        }
    
        void Window::setViewportSize(int width, int height)
        {
            if (width <= 0 || height <= 0) {
                return;
            }
        
            m_viewportWidth = width;
            m_viewportHeight = height;
            m_viewportAspectRatio = static_cast<float>(width) / static_cast<float>(height);
    
            draw_frame.view_projection = glm::ortho(
                0.0f,
                static_cast<float>(m_viewportWidth),
                static_cast<float>(m_viewportHeight),
                0.0f,
                1.0f,
                -1.0f
            );
    
            updateViewport();
        }
    
        glm::vec2 Window::windowToViewport(glm::vec2 windowPos)
        {
            glm::vec4 viewport = draw_frame.viewport;
        
            // check if outside viewport
            if (windowPos.x < viewport.x || windowPos.x > viewport.x + viewport.z ||
                windowPos.y < viewport.y || windowPos.y > viewport.y + viewport.w) {
                return glm::vec2(-1.0f, -1.0f);
            }
    
            float viewport_x = ((windowPos.x - viewport.x) / viewport.z) * m_viewportWidth;
            float viewport_y = ((windowPos.y - viewport.y) / viewport.w) * m_viewportHeight;
    
            return glm::vec2(viewport_x, viewport_y);
        }

    void Window::translateCamera(glm::f32 deltaX, glm::f32 deltaY)
    {
        draw_frame.camera_xform = glm::translate(
            draw_frame.camera_xform, 
            glm::vec3(-deltaX, -deltaY, 0.0f)
        );
    }
    
    void Window::setCameraPosition(glm::vec2 position)
    {
        glm::vec3 scale = glm::vec3(
            draw_frame.camera_xform[0][0],
            draw_frame.camera_xform[1][1],
            draw_frame.camera_xform[2][2]
        );
        glm::mat4 xform = glm::mat4(1.0f);
        xform = glm::scale(xform, scale);
        xform = glm::translate(xform, glm::vec3(-position, 0.0f));
        draw_frame.camera_xform = xform;
    }
    
    glm::vec2 Window::getCameraPosition()
    {
        glm::vec2 scale = glm::vec2(
            draw_frame.camera_xform[0][0],
            draw_frame.camera_xform[1][1]
        );
        glm::vec2 position = glm::vec2(
            -draw_frame.camera_xform[3][0],
            -draw_frame.camera_xform[3][1]
        );
        return position / scale;
    }
    
    void Window::draw_text(const std::string& text, const std::shared_ptr<Font>& font, glm::vec2 position, float depth, glm::vec4 color, float scale, Pivot pivot, bool worldSpace, Sprout::TextJustify justify)
    {
        
        // first pass: get the total sizes
        float totalWidth = 0.0f, maxHeight = 0.0f;
        {
            float temp_x = 0.0f, temp_y = 0.0f;
            
            for (char c : text)
            {
                int char_index = c - 32;
                
                float advance_x = temp_x, advance_y = temp_y;
                stbtt_aligned_quad quad;
                
                stbtt_GetBakedQuad
                (
                    font->bakedChars, 
                    m_fontAtlases[font->fontId].width, 
                    m_fontAtlases[font->fontId].height, 
                    char_index, 
                    &advance_x, 
                    &advance_y, 
                    &quad, 
                    1
                );
                
                temp_x = advance_x;
                temp_y = advance_y;
                
                float charHeight = quad.y1 - quad.y0;
                if (charHeight > maxHeight) maxHeight = charHeight;
            }
            
            totalWidth = temp_x;
        }
        
        glm::mat4 globalXform = glm::mat4(1.0f);
        
        if (worldSpace)
        {
            glm::vec2 adjustedPosition = position;
            if (justify == Sprout::TextJustify::CENTER)
            {
                adjustedPosition.x -= (totalWidth * scale) / 2.0f;
            } 
            else if (justify == Sprout::TextJustify::RIGHT)
            {
                adjustedPosition.x -= totalWidth * scale;
            }
            globalXform = glm::translate(globalXform, glm::vec3(adjustedPosition, 0.0f));
        }
        else 
        {
            glm::vec2 anchor_offset = getAnchorOffset(pivot);
            glm::vec2 pos;
            
            pos.x = (anchor_offset.x != 0) ? -position.x * anchor_offset.x : position.x;
            pos.y = (anchor_offset.y != 0) ? -position.y * anchor_offset.y : -position.y;
            
            pos.x += anchor_offset.x * m_viewportWidth;
            pos.y += anchor_offset.y * m_viewportHeight;
            
            if (justify == Sprout::TextJustify::CENTER)
            {
                pos.x -= (totalWidth * scale) / 2.0f;
            } 
            else if (justify == Sprout::TextJustify::RIGHT)
            {
                pos.x -= totalWidth * scale;
            }
            
            globalXform = glm::translate(globalXform, glm::vec3(pos, 0.0f));
        }


        globalXform = glm::scale(globalXform, glm::vec3(scale, scale, 1.0f));
        
        float x = 0, y = 0;
        
        // second pass: render text with pivot and justify transformations
        for (char c : text)
        {
            int char_index = c - 32;
            
            float advance_x = x, advance_y = y;
            stbtt_aligned_quad quad;
            
            stbtt_GetBakedQuad
            (
                font->bakedChars, 
                m_fontAtlases[font->fontId].width, 
                m_fontAtlases[font->fontId].height, 
                char_index, 
                &advance_x, 
                &advance_y, 
                &quad, 
                1
            );
            
            glm::vec2 size = glm::vec2(quad.x1 - quad.x0, quad.y1 - quad.y0);
            
            glm::mat4 charXform = globalXform;
            charXform = glm::translate(charXform, glm::vec3(quad.x0, quad.y0, 0.0f));
            glm::vec4 uv = glm::vec4(quad.s0, quad.t0, quad.s1, quad.t1);
            
            glm::mat4 projection;
            if (worldSpace)
            {
                projection = draw_frame.view_projection * draw_frame.camera_xform * charXform;            
            }
            else
            {
                projection = draw_frame.view_projection * charXform;
            }
            draw_rect_projected(projection, size, depth, uv, color, Pivot::TOP_LEFT, font->fontId+1);
            
            x = advance_x;
            y = advance_y;
        }
    }

}