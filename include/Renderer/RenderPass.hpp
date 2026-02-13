//
//  RenderPass.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

namespace Sprout
{
    class Window;

    class RenderPass
    {
    public:
        virtual ~RenderPass() = default;
        virtual void execute(Window& window) = 0;
        virtual void clear() {}

        bool enabled = true;
        int priority = 0; // lower runs first
    };
}
