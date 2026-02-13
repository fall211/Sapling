//
//  ImGuiContext.hpp
//  SaplingEngine, Editor
//

#pragma once

struct sapp_event;

class ImGuiContext
{
public:
    static ImGuiContext& getInstance();

    void initialize();
    void cleanup();

    void newFrame();
    void render();
    bool handleEvent(const sapp_event* ev);

    bool wantsCaptureMouse() const;
    bool wantsCaptureKeyboard() const;

private:
    ImGuiContext() = default;
    bool m_initialized = false;
};
