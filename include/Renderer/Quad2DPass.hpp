//
//  Quad2DPass.hpp
//  Sapling Engine, Sprout Renderer
//

#pragma once

#include "Renderer/RenderPass.hpp"

namespace Sprout
{
    class Quad2DPass : public RenderPass
    {
    public:
        Quad2DPass() = default;
        ~Quad2DPass() override = default;

        void execute(Window& window) override;
        void clear() override;
    };
}
