#pragma once

#include "../Graphics.hpp"
#include "xr_struct_mapping.hpp"
#include "VulkanContext.hpp"
#include <vulkan/vulkan.h>
#include <openxr/openxr_platform.h>
#include "openxr_ext_loader.h"
#include "VkEnumerators.hpp"
#include "Memory.hpp"

#include <stdexcept>
#include <sstream>
#include <format>
#include <span>
#include <filesystem>

namespace vr {

    template<typename T>
    struct Link {
        T* cpu{};
        Buffer gpu{};
    };

    struct VulkanSwapChain{
        std::string name;
        XrSwapchain swapchain;
        uint32_t width;
        uint32_t height;
        VkFormat format;
        std::vector<XrSwapchainImageVulkanKHR> images;
    };

    struct CopyRequest {
        Buffer source;
        ImageId imageId;
        uint32_t mipLevel{0};
        uint32_t  arrayLayer{0};
    };

    class VulkanGraphicsService : public GraphicsService {
    public:
        explicit VulkanGraphicsService(const Context &context);

        ~VulkanGraphicsService() override = default;

        void setSwapChains(std::vector<SwapChain> swapchains)  final;

        [[nodiscard]]
        const VulkanContext& vulkanContext() const;

        void init() final;

        [[nodiscard]]
        const XrBaseInStructure& graphicsBinding() const final;

        Buffer createStagingBuffer(VkDeviceSize size);

        Buffer createDeviceLocalBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

        Buffer createMappableBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

        void release(Buffer buffer);

        [[nodiscard]]
        int64_t swapChainFormat() const final;

        [[nodiscard]] VkQueue queue() const;

        [[nodiscard]] VkDevice device() const {
            return m_device;
        }

        const VulkanSwapChain& getSwapChain(const std::string& name);

        void shutdown() override;

        VkCommandBuffer commandBuffer(uint32_t imageIndex);

        static std::shared_ptr<GraphicsService> shared(const Context& context);

        template<typename T = void>
        T* map(Buffer& buffer) {
            vmaMapMemory(allocator.allocator, buffer.allocation, &buffer.mapping);
            return reinterpret_cast<T*>(buffer.mapping);
        }

        void unmap(Buffer& buffer) const {
            vmaUnmapMemory(allocator.allocator, buffer.allocation);
            buffer.mapping = nullptr;
        }

        void scoped(auto&& operation) {
            auto allocateInfo = makeStruct<VkCommandBufferAllocateInfo>();
            allocateInfo.commandPool = m_scopedCommandPool;
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(m_device, &allocateInfo, &commandBuffer);

            auto beginInfo = makeStruct<VkCommandBufferBeginInfo>();
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            operation(commandBuffer);
            vkEndCommandBuffer(commandBuffer);

            auto submitInfo = makeStruct<VkSubmitInfo>();
            submitInfo.pCommandBuffers = &commandBuffer;
            submitInfo.commandBufferCount = 1;

            CHECK_VULKAN(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
            vkQueueWaitIdle(m_graphicsQueue);
            vkFreeCommandBuffers(m_device, m_scopedCommandPool, 1, &commandBuffer);
        }

        VkImageView createImageView(VkImageViewCreateInfo createInfo);

        void copyToImage(const CopyRequest& request);

        void copy(const Buffer& src, const Buffer& dst, VkDeviceSize size, VkDeviceSize offset = 0u, VkDeviceSize dstOffset = 0u);

        template<typename T>
        Link<T> link(VkBufferUsageFlagBits usage, uint32_t count = 1) {
            Link<T> link{};
            link.gpu = createMappableBuffer(sizeof(T) * count, usage);
            link.cpu = map<T>(link.gpu);
            return link;
        }

        template<typename T>
        void unlink(Link<T> link) {
            unmap(link.cpu);
        }

        VkDescriptorPool createDescriptorPool(const VkDescriptorPoolCreateInfo& createInfo);

        VkDescriptorSetLayout createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo createInfo);

        std::vector<VkDescriptorSet> allocate(VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t numSets = 1);

        void update(std::span<VkWriteDescriptorSet> writes);

        std::span<VkCommandBuffer> allocateCommandBuffers(uint32_t size);

        VkShaderModule createShaderModule(const std::filesystem::path &path);

        VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo &createInfo);

        VkPipeline createGraphicsPipeline(const VkGraphicsPipelineCreateInfo &createInfo);

        VkRenderPass createRenderPass(const VkRenderPassCreateInfo &createInfo);

        VkFramebuffer createFrameBuffer(const VkFramebufferCreateInfo &createInfos);

        std::vector<VkFramebuffer> createFrameBuffers(const std::vector<VkFramebufferCreateInfo> &createInfos);

        Image creatImage(const VkImageCreateInfo &createInfo);

        void submitToGraphicsQueue(const VkSubmitInfo &submitInfo);

    private:
        void pickDevice();

        void setupQueues();
        
        void createDevice();

        void createInternalCommandPool();

        void initMemoryAllocator();

        void initializeGraphicsBinding();

        void logDevice();


        void initGuard() const;

    private:
        XrGraphicsBindingVulkanKHR m_bindingInfo{ XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR };
        VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
        VkDevice m_device{VK_NULL_HANDLE};
        VkQueue m_graphicsQueue{VK_NULL_HANDLE};
        uint32_t m_graphicsFamilyIndex{};
        std::vector<VulkanSwapChain> m_swapChains;
        VmaMemoryAllocator allocator;
        VkCommandPool m_commandPool;
        VkCommandPool m_scopedCommandPool;
        static constexpr uint32_t MaxCommandBuffers{100};
        std::vector<VkCommandBuffer> m_commandBuffers;
        uint32_t numCommandBuffers{};
        bool initialized{false};
        std::vector<Buffer> m_buffers;
        std::vector<VkCommandPool> m_commandPools;
        std::vector<VkDescriptorPool> m_descriptorPools;
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<VkPipelineLayout> m_pipelineLayouts;
        std::vector<VkPipeline> m_pipelines;
        std::vector<VkRenderPass> m_renderPasses;
        std::vector<VkFramebuffer> m_frameBuffers;
        std::vector<Image> m_images;
    };
}
