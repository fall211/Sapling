//
//  Quad2DPass.cpp
//  Sapling Engine, Sprout Renderer
//

#include "Renderer/Quad2DPass.hpp"
#include "Renderer/Sprout.hpp"
#include "Renderer/quad.h"

namespace Sprout
{

void Quad2DPass::clear()
{
    // clearing is handled by Window::Frame() since DrawFrame is owned by Window
}

void Quad2DPass::execute(Window& window)
{
    if (!enabled) return;

    auto& state = window.m_state;
    auto& df = window.draw_frame;

    sg_apply_pipeline(state.pip);

    // draw standalone (independent) textures one at a time
    if (df.num_images > 0) {
        sg_update_buffer(state.standalone_vbuf, SG_RANGE(df.standalone_quads));
        state.bind.vertex_buffers[0] = state.standalone_vbuf;

        for (int i = 0; i < df.num_images; i++) {
            if (df.images[i].id != SG_INVALID_ID) {
                state.bind.images[IMG_texture0] = df.images[i];
                sg_apply_bindings(&state.bind);
                sg_draw(i * 6, 6, 1);
            }
        }
    }

    // draw batched atlas quads
    state.bind.images[IMG_texture0] = window.m_atlas.img;
    state.bind.vertex_buffers[0] = state.quad_vbuf;
    state.bind.vertex_buffer_offsets[0] = 0;
    sg_apply_bindings(&state.bind);
    sg_draw(0, 6 * df.num_quads, 1);
}

} // namespace Sprout
