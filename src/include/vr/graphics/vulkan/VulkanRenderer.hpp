#pragma once

#include "vr/graphics/Renderer.hpp"
#include "VulkanGraphicsService.hpp"

namespace vr {

    class VulkanRenderer : public Renderer {
    public:
        ~VulkanRenderer() override = default;

        VulkanGraphicsService& graphicsService() {
            return *reinterpret_cast<VulkanGraphicsService*>(m_graphics.get());
        }
    };
}