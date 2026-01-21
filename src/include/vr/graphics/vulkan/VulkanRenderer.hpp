#pragma once

#include "vr/graphics/Renderer.hpp"
#include "VulkanGraphicsService.hpp"

namespace vr {

    class VulkanRenderer : public Renderer {
    public:
        ~VulkanRenderer() override = default;

        void paused(const FrameInfo &frameInfo, Layers &layers) override {
            render(frameInfo, layers);
        }

        VulkanGraphicsService& graphicsService() {
            return *reinterpret_cast<VulkanGraphicsService*>(m_graphics.get());
        }
    };
}