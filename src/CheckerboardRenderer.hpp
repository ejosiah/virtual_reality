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
        auto mapping = graphicsService().map(buffer);
        auto colors = mapping.as<Color>();
        const auto w = int(swapChain.width);
        const auto h = int(swapChain.height);

        for (auto layerId = 0u; layerId < m_layers.size(); layerId++) {
            auto& layer = m_layers[layerId];
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
            for(auto imageIndex = 0u; imageIndex < swapChain.images.size(); imageIndex++) {
                graphicsService().copyToImage({ buffer, {swapChain._, imageIndex}, 0, layerId });
            }

            layer.eyeVisibility = static_cast<XrEyeVisibility>(layerId);
            layer.subImage.imageArrayIndex = layerId;
            layer.subImage.swapchain = swapChain._;
            layer.subImage.imageRect = {{0, 0}, static_cast<int32_t>(swapChain.width),
                                                 static_cast<int32_t>(swapChain.height)};
            layer.pose.position = {0, 0, -1};
            layer.pose.orientation = {0, 0, 0, 1};
            layer.size = {1, 1};
        }

        graphicsService().release(buffer);
    }


    void paused(const vr::FrameInfo &frameInfo, vr::Layers& layers) override {
        return render(frameInfo, layers);
    }

    void render(const vr::FrameInfo &frameInfo, vr::Layers& layers) override {
        layers.push_back({&m_layers[0]});
        layers.push_back({&m_layers[1]});
    }

    static std::shared_ptr<vr::Renderer> shared() {
        return std::make_shared<CheckerboardRenderer>();
    }

    static vr::SessionConfig session() {
        return  vr::SessionConfig{}.addSwapChain(
                    vr::SwapchainSpecification()
                        .name("Checkerboard")
                        .usage()
                            .colorAttachment()
                        .format(VK_FORMAT_R8G8B8A8_SRGB)
                        .arraySize(2)
                        .width(2048)
                        .height(2048)
                    ).addSwapChain(
                        vr::SwapchainSpecification()
                            .name("depth_buffer")
                            .usage()
                                .depthStencilAttachment()
                            .format(VK_FORMAT_D32_SFLOAT_S8_UINT)
                            .arraySize(2)
                            .width(2048)
                            .height(2048)
                    );
    }

    vr::cstring name() override {
        return "Checkerboard";
    }

private:
    std::vector<XrCompositionLayerQuad> m_layers{
            {XR_TYPE_COMPOSITION_LAYER_QUAD},
            {XR_TYPE_COMPOSITION_LAYER_QUAD}};
};