#pragma once


#include "vr/graphics/vulkan/VulkanRenderer.hpp"
#include "vr/SessionConfig.hpp"
#include "util/collections.hpp"
#include "util/types.hpp"

#include <stb_image.h>

enum class EnvType {
    CUBE_MAP, EQUI_RECTANGULAR
};

class EnvironmentRenderer : public vr::VulkanRenderer {
public:
    ~EnvironmentRenderer() override = default;

    void init() override {
        loadCubeMap();
        loadEquirectMap();
        m_frame.layers.resize(1);
    }

    void loadCubeMap() {
        const auto& swapChain = graphicsService().getSwapChain("skybox");
        m_cubemapLayer.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
        m_cubemapLayer.swapchain = swapChain._;
        m_cubemapLayer.orientation = {0, 0, 0, 1};
        m_cubemapLayer.imageArrayIndex = 0;
        m_layers[0] = reinterpret_cast<XrCompositionLayerBaseHeader*>(&m_cubemapLayer);

        VkDeviceSize size = swapChain.width * swapChain.height * 4;
        VkDeviceSize offset = 0;
        auto staging = graphicsService().createStagingBuffer(size * faces.size());
        auto mapping = graphicsService().map(staging);

        std::array<VkBufferImageCopy, 6> regions{};

        for(auto face = 0u; face < 6; face++) {
            auto path = std::format("{}\\{}.jpg", rootPath, faces[face]);
            int w, h, c;
            auto image = stbi_load(path.c_str(), &w, &h, &c, STBI_rgb_alpha);
            auto dst = reinterpret_cast<stbi_uc*>(mapping._) + offset;
            std::memcpy(dst, image, size);
            stbi_image_free(image);
            regions[face].bufferOffset = offset;
            regions[face].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            regions[face].imageSubresource.mipLevel = 0;
            regions[face].imageSubresource.baseArrayLayer = face;
            regions[face].imageSubresource.layerCount = 1;
            regions[face].imageOffset = {0, 0, 0};
            regions[face].imageExtent = {static_cast<uint32_t>(w), static_cast<uint32_t>(h), 1u};
            offset += size;
        }
//        mapping.unmap();

        graphicsService().scoped([&](auto commandBuffer) {

            for(auto image : swapChain.images) {
                auto barrier = makeStruct<VkImageMemoryBarrier>();
                barrier.srcAccessMask = VK_ACCESS_NONE;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.image = image.image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 6;

                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,nullptr, 0, nullptr, 1, &barrier);

                vkCmdCopyBufferToImage(commandBuffer, staging._, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());

                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_NONE;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,nullptr, 0, nullptr, 1, &barrier);
            }
        });
    }

    void loadEquirectMap() {
        int width, height, comps;
        stbi_info(equirectPath2, &width, &height, &comps);
        const auto& swapChain = graphicsService().getSwapChain("equi_rectangular0");
        m_equiRectLayer.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
        m_equiRectLayer.subImage.swapchain = swapChain._;
        m_equiRectLayer.subImage.imageArrayIndex = 0;
        m_equiRectLayer.subImage.imageRect = { width, height };
        m_equiRectLayer.pose.orientation = { 0, 0, 0, 1 };
        m_equiRectLayer.pose.position = {0, 0, 0};
        m_equiRectLayer.radius = 0;
        m_equiRectLayer.scale = { 1, 1};
        m_equiRectLayer.bias = {0, 0};
        m_layers[1] = reinterpret_cast<XrCompositionLayerBaseHeader*>(&m_equiRectLayer);

        uint32_t numPixels = swapChain.width * swapChain.height * STBI_rgb_alpha;
        VkDeviceSize size = numPixels * sizeof(uint16_t);
        auto staging = graphicsService().createStagingBuffer(size);
        auto mapping = graphicsService().map(staging);
        auto dst = mapping.as<uint16_t>();

        auto hdr_image = stbi_loadf(equirectPath2, &width, &height, &comps, STBI_rgb_alpha);
        auto hdr_uint32 = reinterpret_cast<uint32_t*>(hdr_image);

        for(int i = 0; i < numPixels; i++){
            dst[i] = float_to_half_branch(hdr_uint32[i]);
        }
        stbi_image_free(hdr_image);
        
        graphicsService().scoped([&](auto commandBuffer){

            VkBufferImageCopy region{0, 0, 0};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u};

            for(auto image : swapChain.images) {
                auto barrier = makeStruct<VkImageMemoryBarrier>();
                barrier.srcAccessMask = VK_ACCESS_NONE;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.image = image.image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,nullptr, 0, nullptr, 1, &barrier);

                vkCmdCopyBufferToImage(commandBuffer, staging._, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_NONE;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,nullptr, 0, nullptr, 1, &barrier);
            }
        });
    }

    std::vector<vr::Vibrate> set(const vr::ActionSet &actionSet) override {
        if(actionSet.get("a").value<bool>()){
            currentLayerIndex = (currentLayerIndex + 1) % 2;
        }
        return {};
    }

    vr::cstring name() override {
        return "Environment";
    }

    vr::FrameEnd paused(const vr::FrameInfo &frameInfo) override {
        return render(frameInfo);
    }

    vr::FrameEnd render(const vr::FrameInfo &frameInfo) override {
        m_frame.layers[0] = m_layers[currentLayerIndex];
        m_frame.layers[0]->space = frameInfo.space;
        return m_frame;
    }

    static std::shared_ptr<Renderer> shared() {
        return std::make_shared<EnvironmentRenderer>();
    }

    static vr::SessionConfig session() {
        int width, height, comps;
        stbi_info(std::format("{}\\{}", rootPath, "left.jpg").c_str(), &width, &height, &comps);
        spdlog::info("cubemap: width: {}, height: {}, channels: {}", width, height, comps);

        int eWidth, eHeight;
        stbi_info(equirectPath2, &eWidth, &eHeight, &comps);
        spdlog::info("equi rectangular map: width: {}, height: {}, channels: {}", width, height, comps);

        return
            vr::SessionConfig()
                .baseSpaceType().local()
                .addActionSet(
                        vr::ActionSetSpecification()
                            .name("main")
                            .addAction("a", XR_ACTION_TYPE_BOOLEAN_INPUT, "/user/hand/right/input/a/click")
                        )
                .addSwapChain(
                    vr::SwapchainSpecification()
                        .name("skybox")
                        .usage()
                            .colorAttachment()
                            .transferDestination()
                            .transferSource()
                        .format(VK_FORMAT_R8G8B8A8_SRGB)
                        .faceCount(6)
                        .arraySize(1)
                        .width(width)
                        .height(height)
                        )
                .addSwapChain(
                    vr::SwapchainSpecification()
                        .name("equi_rectangular0")
                        .usage()
                            .colorAttachment()
                            .transferDestination()
                            .transferSource()
                        .format(VK_FORMAT_R16G16B16A16_SFLOAT)
                        .faceCount(1)
                        .arraySize(1)
                        .width(eWidth)
                        .height(eHeight)
                        )
                .addSwapChain(
                    vr::SwapchainSpecification()
                        .name("equi_rectangular1")
                        .usage()
                            .colorAttachment()
                            .transferDestination()
                            .transferSource()
                        .format(VK_FORMAT_R16G16B16A16_SFLOAT)
                        .faceCount(1)
                        .arraySize(1)
                        .width(eWidth)
                        .height(eHeight)
                        )
        ;
    }
    
private:
    static constexpr vr::cstring rootPath{R"(C:\Users\Josiah Ebhomenye\OneDrive\media\textures\skybox\005)"};
    static constexpr vr::cstring equirectPath1{R"(C:\Users\Josiah Ebhomenye\Downloads\meadow_4k.hdr)"};
    static constexpr vr::cstring equirectPath2{R"(C:\Users\Josiah Ebhomenye\Downloads\small_empty_room_3_4k.hdr)"};
    static constexpr std::array<vr::cstring, 6> faces {
        "right", "left", "top", "bottom", "front", "back"
    };

    XrCompositionLayerCubeKHR m_cubemapLayer{ XR_TYPE_COMPOSITION_LAYER_CUBE_KHR };
    XrCompositionLayerEquirectKHR m_equiRectLayer{ XR_TYPE_COMPOSITION_LAYER_EQUIRECT_KHR };
    vr::FrameEnd m_frame{};
    std::array<XrCompositionLayerBaseHeader*, 2> m_layers;
    int currentLayerIndex{};
    EnvType envType{EnvType::CUBE_MAP};
};