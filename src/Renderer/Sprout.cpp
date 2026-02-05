//
//  Sprout.cpp
//  Sapling Engine, Sprout Renderer
//

#include "Renderer/StandaloneTexture.hpp"
#include "Utility/Debug.hpp"
#include <cstddef>
#if defined(__APPLE__)
    #define SOKOL_METAL
#elif defined(_WIN32)
    #define SOKOL_D3D11
    #define SOKOL_IMPL
#else
    #define SOKOL_GLCORE
    #define SOKOL_IMPL
#endif

#define SOKOL_NO_ENTRY
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_glue.h"


#include "Renderer/Sprout.hpp"
#include "Utility/Color.hpp"
#include "Renderer/quad.h"

#include <algorithm>
#include <iostream>

namespace Sprout 
{
    glm::vec2 getPivotOffset(Pivot pivot)
    {
        switch (pivot) {
            case Pivot::TOP_LEFT:   return glm::vec2(0.0f, 0.0f);
            case Pivot::TOP_CENTER: return glm::vec2(0.5f, 0.0f);
            case Pivot::TOP_RIGHT:  return glm::vec2(1.0f, 0.0f);
            case Pivot::CENTER_LEFT:   return glm::vec2(0.0f, 0.5f);
            case Pivot::CENTER:        return glm::vec2(0.5f, 0.5f);
            case Pivot::CENTER_RIGHT:  return glm::vec2(1.0f, 0.5f);
            case Pivot::BOTTOM_LEFT:      return glm::vec2(0.0f, 1.0f);
            case Pivot::BOTTOM_CENTER:    return glm::vec2(0.5f, 1.0f);
            case Pivot::BOTTOM_RIGHT:     return glm::vec2(1.0f, 1.0f);
            default:                   return glm::vec2(0.5f, 0.5f); // default to center
        }
    }
    
    glm::vec2 getAnchorOffset(Pivot pivot)
    {
        switch (pivot) {
            case Pivot::TOP_LEFT:   return glm::vec2(0.0f, 0.0f);
            case Pivot::TOP_CENTER: return glm::vec2(0.5f, 0.0f);
            case Pivot::TOP_RIGHT:  return glm::vec2(1.0f, 0.0f);
            case Pivot::CENTER_LEFT:   return glm::vec2(0.0f, 0.5f);
            case Pivot::CENTER:        return glm::vec2(0.5f, 0.5f);
            case Pivot::CENTER_RIGHT:  return glm::vec2(1.0f, 0.5f);
            case Pivot::BOTTOM_LEFT:      return glm::vec2(0.0f, 1.0f);
            case Pivot::BOTTOM_CENTER:    return glm::vec2(0.5f, 1.0f);
            case Pivot::BOTTOM_RIGHT:     return glm::vec2(1.0f, 1.0f);
            default:                   return glm::vec2(0.0f, 0.0f); // default to top left
        }
    }
    
    
    Window::Window(int viewportWidth, int viewportHeight, const char* title)
        :   m_title(title),
            m_viewportWidth(viewportWidth),
            m_viewportHeight(viewportHeight)
        {    
            memset(&m_state, 0, sizeof(m_state));
            memset(&draw_frame, 0, sizeof(draw_frame));
    
            m_viewportAspectRatio = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
            
            // set the projection to the viewport (will update once the viewport gets scaled to window)
            draw_frame.view_projection = glm::ortho(
                0.0f,
                static_cast<float>(m_viewportWidth),
                static_cast<float>(m_viewportHeight),
                0.0f,
                1.0f,
                -1.0f
            );
            
            draw_frame.camera_xform = glm::mat4(1.0f);

            // Initialize viewport dimensions (will be updated in Init())
            draw_frame.viewport = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

            Instance = this;
        }
    
    Window::~Window()
    {
        delete Instance;
    }
    
    void Window::init_cb()
    {
        Instance->Init();
    }
    
    void Window::frame_cb()
    {
        Instance->Frame();
    }
    
    void Window::cleanup_cb()
    {
        Instance->Cleanup();
    }
    
    void Window::event_cb(const sapp_event* e)
    {
        Instance->Event(e);
    }
    
    
    void Window::Init()
    {
        updateViewport();
        
        sg_desc desc = {};
        desc.environment = sglue_environment();   
        sg_setup(&desc);


        // main vertex buffer for regular quads
        sg_buffer_desc vbuf_desc = {};
        vbuf_desc.size = sizeof(draw_frame.quads);
        vbuf_desc.usage = SG_USAGE_DYNAMIC;
        vbuf_desc.label = "quad-vertices";
        
        m_state.quad_vbuf = sg_make_buffer(&vbuf_desc);
        
        // separate vertex buffer for standalone quads
        sg_buffer_desc standalone_vbuf_desc = {};
        standalone_vbuf_desc.size = sizeof(draw_frame.standalone_quads);
        standalone_vbuf_desc.usage = SG_USAGE_DYNAMIC;
        standalone_vbuf_desc.label = "standalone-quad-vertices";
        
        m_state.standalone_vbuf = sg_make_buffer(&standalone_vbuf_desc);
        
        // index buffer
        const int INDEX_BUFFER_COUNT = MAX_QUADS * 6 + MAX_STANDALONE_TEXTURES * 6;
        std::array<glm::u16, INDEX_BUFFER_COUNT> indices;
        
        for (int i = 0; i < INDEX_BUFFER_COUNT;)
        {
            indices.at(i + 0) = ((i/6)*4 + 0);
    		indices.at(i + 1) = ((i/6)*4 + 1);
    		indices.at(i + 2) = ((i/6)*4 + 2);
    		indices.at(i + 3) = ((i/6)*4 + 0);
    		indices.at(i + 4) = ((i/6)*4 + 2);
    		indices.at(i + 5) = ((i/6)*4 + 3);
    		i += 6;
        }
        
        sg_buffer_desc ibuf_desc = {};
        ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
        ibuf_desc.size = sizeof(indices);
        ibuf_desc.data = SG_RANGE(indices);
        ibuf_desc.label = "quad-indices";
        
        m_state.bind.index_buffer = sg_make_buffer(&ibuf_desc);
        
        sg_sampler_desc sampler_desc = {};
        sampler_desc.min_filter = SG_FILTER_NEAREST;
        sampler_desc.mag_filter = SG_FILTER_NEAREST;
        sampler_desc.mipmap_filter = SG_FILTER_NEAREST;
        sampler_desc.wrap_u = SG_WRAP_REPEAT;
        sampler_desc.wrap_v = SG_WRAP_REPEAT;
        
        m_state.bind.samplers[SMP_default_sampler] = sg_make_sampler(&sampler_desc);
        
        // pipeline
        sg_shader shd = sg_make_shader(quad_shader_desc(sg_query_backend()));
        
        sg_pipeline_desc pip_desc = {};
        pip_desc.shader = shd;
        pip_desc.index_type = SG_INDEXTYPE_UINT16;
        pip_desc.layout.attrs[ATTR_quad_position0].format = SG_VERTEXFORMAT_FLOAT4;
        pip_desc.layout.attrs[ATTR_quad_color0].format = SG_VERTEXFORMAT_FLOAT4;
        pip_desc.layout.attrs[ATTR_quad_uv0].format = SG_VERTEXFORMAT_FLOAT2;
        pip_desc.layout.attrs[ATTR_quad_color_override0].format = SG_VERTEXFORMAT_FLOAT4;
        pip_desc.layout.attrs[ATTR_quad_bytes0].format = SG_VERTEXFORMAT_UBYTE4N;
        pip_desc.label = "quad-pipeline";

        
        sg_blend_state blend_state = {};
        blend_state.enabled = true;
        blend_state.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
        blend_state.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend_state.op_rgb = SG_BLENDOP_ADD;
        blend_state.src_factor_alpha = SG_BLENDFACTOR_ONE;
        blend_state.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend_state.op_alpha = SG_BLENDOP_ADD;
        
        pip_desc.colors[0].blend = blend_state;    
        
        
        m_state.pip = sg_make_pipeline(&pip_desc);
        
        sg_pass_action pass_action = {};
        pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
        pass_action.colors[0].clear_value = {0.0f, 0.0f, 0.0f, 1.0f};

        m_state.pass_action = pass_action;


        // bake images into atlas
        bake_atlas();
        init_fonts();
    }
    
    bool sortByZ(const Quad& a, const Quad& b)
    {
        return a.vertices[0].pos.z > b.vertices[0].pos.z;
    }
    
    void Window::Frame()
    {
        // reset draw frame
        draw_frame.num_quads = 0;
        std::fill(draw_frame.quads.begin(), draw_frame.quads.end(), Quad{});
        
        draw_frame.num_images = 0;
        std::fill(draw_frame.standalone_quads.begin(), draw_frame.standalone_quads.end(), Quad{});
        for (size_t i = 0; i < MAX_STANDALONE_TEXTURES; i++)
        {
            draw_frame.images[i].id = SG_INVALID_ID;
        }
        
        // delta time calculation (not smoothed like sapp_frame_duration())
        // should be running at constant 60 fps, but just in case
        auto now = std::chrono::system_clock::now();
        m_delta_time = std::chrono::duration<double>(now - m_last_frame_time).count();
        m_last_frame_time = now;
        
        if (m_update_frame_callback) 
        {
            m_update_frame_callback(m_delta_time);
        }
        
        // sort quads by z so we have layers (z buffer alternative to keep transparency)
        std::sort(draw_frame.quads.begin(), draw_frame.quads.begin() + draw_frame.num_quads, sortByZ);
        std::sort(draw_frame.standalone_quads.begin(), draw_frame.standalone_quads.begin() + draw_frame.num_images, sortByZ);
        
        m_state.bind.images[IMG_fontTex1] = m_fontAtlases[0].img;

        sg_update_buffer(m_state.quad_vbuf, SG_RANGE(draw_frame.quads));

        // only called once
        sg_pass pass = {};
        pass.action = m_state.pass_action;

        pass.swapchain = sglue_swapchain();
        sg_begin_pass(&pass);
        
        glm::vec4 viewport = draw_frame.viewport;
        sg_apply_viewport((int)viewport.x, (int)viewport.y, (int)viewport.z, (int)viewport.w, true);
        sg_apply_scissor_rect((int)viewport.x, (int)viewport.y, (int)viewport.z, (int)viewport.w, true);
        
        sg_apply_pipeline(m_state.pip);

        // draw standalone textures
        if (draw_frame.num_images > 0) {
            sg_update_buffer(m_state.standalone_vbuf, SG_RANGE(draw_frame.standalone_quads));
            m_state.bind.vertex_buffers[0] = m_state.standalone_vbuf;

            for (int i = 0; i < draw_frame.num_images; i++) {
                if (draw_frame.images[i].id != SG_INVALID_ID) {
                    m_state.bind.images[IMG_texture0] = draw_frame.images[i];
                    sg_apply_bindings(&m_state.bind);
                    sg_draw(i * 6, 6, 1);
                }
            }
        }
        m_state.bind.images[IMG_texture0] = m_atlas.img;
        m_state.bind.vertex_buffers[0] = m_state.quad_vbuf;
        m_state.bind.vertex_buffer_offsets[0] = 0;
        sg_apply_bindings(&m_state.bind);
        sg_draw(0, 6 * draw_frame.num_quads, 1);
        


        // called once
        sg_end_pass();
        sg_commit();
    }
    
    void Window::Cleanup()
    {
        sg_shutdown();
    }
    
    void Window::Event(const sapp_event* e)
    {
        
        if (m_event_callback) 
        {
            m_event_callback(e);
        }
        // default behavior
        {
            if (e->type == SAPP_EVENTTYPE_KEY_DOWN) 
            {   
                if (e->key_code == SAPP_KEYCODE_ESCAPE) 
                {
                    // sapp_request_quit();
                }
            }
            else if (e->type == SAPP_EVENTTYPE_RESIZED)
            {
                updateViewport();
            }
        }
    }
        
    auto Window::sokol_main() -> sapp_desc 
    {
        sapp_desc desc = {};
        desc.init_cb = init_cb;
        desc.frame_cb = frame_cb;
        desc.cleanup_cb = cleanup_cb;
        desc.event_cb = event_cb;
        desc.high_dpi = true;
        desc.fullscreen = true;
        desc.window_title = Instance->m_title;
        return desc;
    }
    
    Sprout::Window* Window::Instance = nullptr;
    
    void Window::Run()
    {
        sapp_run(sokol_main());
    }


} // namespace Sprout