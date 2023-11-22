#pragma once

#include <vr/graphics/vulkan/VulkanRenderer.hpp>
#include "vr/SessionConfig.hpp"
#include <glm/glm.hpp>

struct Color {
    char r, g, b, a;
};

struct CheckerboardRenderer : public vr::VulkanRenderer {

    CheckerboardRenderer() = default;

    ~CheckerboardRenderer() override = default;

    static glm::vec4 coloredCheckerboard(const glm::vec2& uv) {
        using namespace glm;
        float x = 2.f * uv.x - 1.f;
        float y = 2.f * uv.y - 1.f;

        return {
                step(1.0, mod(floor((x + 1.0) / 0.2) + floor((y + 1.0) / 0.2), 2.0)),
                step(1.0, mod(floor((x + 1.0) / 0.3) + floor((y + 1.0) / 0.3), 2.0)),
                step(1.0, mod(floor((x + 1.0) / 0.4) + floor((y + 1.0) / 0.4), 2.0)), 1};
    }

    static glm::vec4 checkerboard(const glm::vec2& uv) {
        using namespace glm;
        float x = 2.f * uv.x - 1.f;
        float y = 2.f * uv.y - 1.f;

        return {
                step(1.0, mod(floor((x + 1.0) / 0.2) + floor((y + 1.0) / 0.2), 2.0)),
                step(1.0, mod(floor((x + 1.0) / 0.2) + floor((y + 1.0) / 0.2), 2.0)),
                step(1.0, mod(floor((x + 1.0) / 0.2) + floor((y + 1.0) / 0.2), 2.0)), 1};
    }

    void init() override {
        using namespace glm;
        const auto& swapChain = graphicsService().getSwapChain("Checkerboard");

        VkDeviceSize size = swapChain.width * swapChain.height * 4;
        auto buffer = graphicsService().createStagingBuffer(size);
        auto colors = graphicsService().map<Color>(buffer);
        const auto w = int(swapChain.width);
        const auto h = int(swapChain.height);

        for (auto layerId = 0; layerId < layers.size(); layerId++) {
            auto& layer = layers[layerId];
            for (auto i = 0; i < h; i++) {
                for (auto j = 0; j < w; j++) {
                    vec2 uv{
                            static_cast<float>(j) / static_cast<float>(w),
                            static_cast<float>(i) / static_cast<float>(h),
                    };
                    int id = i * w + j;

                    vec4 col = layerId == 0 ? checkerboard(uv) : coloredCheckerboard(uv);


                    colors[id].r = static_cast<char>(col.r * 255);
                    colors[id].g = static_cast<char>(col.g * 255);
                    colors[id].b = static_cast<char>(col.b * 255);
                    colors[id].a = static_cast<char>(col.a * 255);
                }
            }
            graphicsService().scoped([&](VkCommandBuffer commandBuffer) {
                for (const auto &image: swapChain.images) {
                    auto barrier = makeStruct<VkImageMemoryBarrier>();
                    barrier.srcAccessMask = VK_ACCESS_NONE;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    barrier.image = image.image;
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = layerId;
                    barrier.subresourceRange.layerCount = 1;

                    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                         nullptr, 0, nullptr, 1, &barrier);

                    VkBufferImageCopy region{0, 0, 0};
                    region.imageOffset = {0, 0, 0};
                    region.imageExtent = {swapChain.width, swapChain.height, 1};
                    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.imageSubresource.mipLevel = 0;
                    region.imageSubresource.baseArrayLayer = layerId;
                    region.imageSubresource.layerCount = 1;
                    vkCmdCopyBufferToImage(commandBuffer, buffer.handle, image.image,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_NONE;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                                         nullptr, 0, nullptr, 1, &barrier);


                }
            });

            layer.eyeVisibility = static_cast<XrEyeVisibility>(layerId);
            layer.subImage.imageArrayIndex = layerId;
            layer.subImage.swapchain = swapChain.swapchain;
            layer.subImage.imageRect = {{0, 0}, static_cast<int32_t>(swapChain.width),
                                                 static_cast<int32_t>(swapChain.height)};
            layer.pose.position = {0, 0, -1};
            layer.pose.orientation = {0, 0, 0, 1};
            layer.size = {1, 1};
            m_frameEnd.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader *>(&layer));
        }

        graphicsService().unmap(buffer);
        graphicsService().release(buffer);
    }


    vr::FrameEnd paused(const vr::FrameInfo &frameInfo) override {
        return render(frameInfo);
    }

    vr::FrameEnd render(const vr::FrameInfo &frameInfo) override {
        for(auto& layer : m_frameEnd.layers) {
            layer->space = frameInfo.cameraSpace;
        }
        return m_frameEnd;
    }

    static std::shared_ptr<vr::Renderer> shared() {
        return std::make_shared<CheckerboardRenderer>();
    }

private:
    std::vector<XrCompositionLayerQuad> layers{
            {XR_TYPE_COMPOSITION_LAYER_QUAD},
            {XR_TYPE_COMPOSITION_LAYER_QUAD}};
    vr::FrameEnd m_frameEnd{};
};