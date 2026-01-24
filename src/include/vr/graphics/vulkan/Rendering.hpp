#pragma once

#include "Memory.hpp"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <optional>
#include <functional>

namespace vr {

    using Scene = std::function<void()>;

    struct Pipeline {
        VkPipeline handle{};
        VkPipelineLayout layout{};
    };

    struct ColorAttachment {
        ImageView imageView;
        VkFormat format;
        glm::vec4 clearValue{0, 0, 0, 1};
        std::optional<ImageView> resolve;
        bool clear{true};
    };

    struct DepthStencilAttachment {
        ImageView imageView;
        VkFormat format;
        glm::vec2 clearValue{1, 0};
        bool clear{true};
    };

    struct RenderInfo {
        std::vector<ColorAttachment> colorAttachments;
        std::optional<DepthStencilAttachment> depthAttachment;
        std::optional<DepthStencilAttachment> stencilAttachment;
        glm::uvec2 renderArea{};
        uint32_t numLayers{1};
        uint32_t viewMask{};
    };


    void render(VkCommandBuffer commandBuffer, const RenderInfo& renderInfo, Scene scene);
}