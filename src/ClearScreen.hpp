#pragma once

#include <vr/graphics/vulkan/VulkanRenderer.hpp>
#include "vr/SessionConfig.hpp"
#include <glm/glm.hpp>
#include <array>

struct aColor {
    char r, g, b, a;
};

struct ClearScreen : public vr::VulkanRenderer {

    ~ClearScreen() override = default;

    void init() override {
        layerViews = decltype(layerViews)(2, makeStruct<XrCompositionLayerProjectionView>());
        clearColors[0].color = {1.f, 0, 0, 1.f};
        clearColors[1].color = {0, 1.f, 0, 1.f};
        clearColors[2].color = {0, 0, 1.f, 1.f};
        createImageViews();

        glm::mat4 result{1};
        result[0];
    }

    void createImageViews() {
        auto swapChain = graphicsService().getSwapChain("main");
        VkImageViewCreateInfo createInfo = makeStruct<VkImageViewCreateInfo>();
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChain.format;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        for(auto image : swapChain.images){
            createInfo.image = image.image;
            auto imageView = graphicsService().createImageView(createInfo);
            imageViews.push_back(imageView);
        }
    }

    vr::FrameEnd paused(const vr::FrameInfo &frameInfo) override {
        return render(frameInfo);
    }

    vr::FrameEnd render(const vr::FrameInfo &frameInfo) override {
        static float elapsedTimeSeconds = 0;
        elapsedTimeSeconds += static_cast<float>(frameInfo.predictedDuration) * 1E-9f;
        const auto swapChain = graphicsService().getSwapChain("main");
        int colorIndex = static_cast<int>(elapsedTimeSeconds/5)%3;
        graphicsService().scoped([&](auto cmdBuffer){
            auto info = makeStruct<VkRenderingInfo>();
            info.flags = VK_RENDERING_CONTENTS_INLINE_BIT_EXT;
            info.renderArea = {{0, 0}, {swapChain.width, swapChain.height}};
            info.layerCount = 1;
            info.colorAttachmentCount = 1;

            auto attachmentInfo = makeStruct<VkRenderingAttachmentInfo>();
            attachmentInfo.imageView = imageViews[frameInfo.imageId.imageIndex];
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentInfo.clearValue = clearColors[colorIndex];
            info.pColorAttachments = &attachmentInfo;

            vkCmdBeginRendering(cmdBuffer, &info);

            vkCmdEndRendering(cmdBuffer);
        });
        for(int i = 0; i < 2; i++){
            auto& view = layerViews[i];
            view.pose = frameInfo.viewInfo.views[i].pose;
            view.fov = frameInfo.viewInfo.views[i].fov;
            view.subImage.swapchain = swapChain._;
            view.subImage.imageRect = {{0, 0}, static_cast<int32_t>(swapChain.width), static_cast<int32_t>(swapChain.height)};
            view.subImage.imageArrayIndex = 0;
        }
        layer.viewCount = 2;
        layer.space = frameInfo.space;
        layer.views = layerViews.data();

        vr::FrameEnd frameEnd{};
        frameEnd.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer));
        return frameEnd;
    }

    void cleanup() override {

    }

    static std::shared_ptr<vr::Renderer> shared() {
        return std::make_shared<ClearScreen>();
    }

    static vr::SessionConfig session() {
        return
                vr::SessionConfig()
                    .addSwapChain(
                        vr::SwapchainSpecification()
                            .name("main")
                            .usage()
                                .colorAttachment()
                                .transferDestination()
                                .transferSource()
                            .format(VK_FORMAT_R8G8B8A8_SRGB)
                            .width(2064)
                            .height(2096));
    }

    vr::cstring name() override {
        return "Clear screen";
    }


private:
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> frameBuffers{};
    XrCompositionLayerProjection layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    std::vector<XrCompositionLayerProjectionView> layerViews;
    VkRenderPass renderPass{};
    std::array<VkClearValue, 3> clearColors{};
};