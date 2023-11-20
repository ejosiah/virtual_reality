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

namespace vr {

    struct VulkanSwapChain{
        std::string name;
        XrSwapchain swapchain;
        std::vector<XrSwapchainImageVulkanKHR> images;
    };

    class VulkanGraphicsService : public GraphicsService {
    public:
        explicit VulkanGraphicsService(const Context &context);

        ~VulkanGraphicsService() override = default;

        void setSwapChains(std::vector<SwapChain> swapchains)  final;

        [[nodiscard]]
        const VulkanContext& vulkanContext() const;
        void init() final;

        [[nodiscard]] const XrBaseInStructure& graphicsBinding() const final;

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

        void shutdown() override {
            allocator.destroy();
        }

        VkCommandBuffer commandBuffer(uint32_t imageIndex);

        static std::shared_ptr<GraphicsService> shared(const Context& context);

    private:
        void pickDevice();

        void setupQueues();
        
        void createDevice();

        void createCommandPool();

        void createCommandBuffers();

        void initMemoryAllocator();

        void initializeGraphicsBinding();

        void logDevice();


        void initGuard() const {
            if(!initialized){
                throw std::runtime_error{"init has not yet been called on GraphicsService"};
            }
        }


    private:
        XrGraphicsBindingVulkanKHR m_bindingInfo{makeStruct<XrGraphicsBindingVulkanKHR>()};
        VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
        VkDevice m_device{VK_NULL_HANDLE};
        VkQueue m_graphicsQueue{VK_NULL_HANDLE};
        uint32_t m_graphicsFamilyIndex{};
        std::vector<VulkanSwapChain> m_swapChains;
        VmaMemoryAllocator allocator;
        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_commandBuffers;
        bool initialized{false};
        std::vector<Buffer> m_buffers;
    };
}
