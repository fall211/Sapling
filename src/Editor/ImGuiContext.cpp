//
//  ImGuiContext.cpp
//  SaplingEngine, Prefab Editor
//

#include "Editor/ImGuiContext.hpp"

#include "imgui/imgui.h"

#if defined(__APPLE__)
    #define SOKOL_METAL
#elif defined(_WIN32)
    #define SOKOL_D3D11
#else
    #define SOKOL_GLCORE
#endif

#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"

#define SOKOL_IMGUI_IMPL
#include "sokol/util/sokol_imgui.h"

ImGuiContext& ImGuiContext::getInstance()
{
    static ImGuiContext instance;
    return instance;
}

void ImGuiContext::initialize()
{
    if (m_initialized) return;

    simgui_desc_t desc = {};
    simgui_setup(&desc);

    m_initialized = true;
}

void ImGuiContext::cleanup()
{
    if (!m_initialized) return;
    simgui_shutdown();
    m_initialized = false;
}

void ImGuiContext::newFrame()
{
    if (!m_initialized) return;

    simgui_frame_desc_t desc = {};
    desc.width = sapp_width();
    desc.height = sapp_height();
    desc.delta_time = sapp_frame_duration();
    desc.dpi_scale = sapp_dpi_scale();
    simgui_new_frame(&desc);
}

void ImGuiContext::render()
{
    if (!m_initialized) return;
    simgui_render();
}

bool ImGuiContext::handleEvent(const sapp_event* ev)
{
    if (!m_initialized) return false;
    return simgui_handle_event(ev);
}

bool ImGuiContext::wantsCaptureMouse() const
{
    if (!m_initialized) return false;
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiContext::wantsCaptureKeyboard() const
{
    if (!m_initialized) return false;
    return ImGui::GetIO().WantCaptureKeyboard;
}
