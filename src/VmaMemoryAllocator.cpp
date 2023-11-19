#include "check.hpp"
#include "vr/graphics/vulkan/Memory.hpp"

#include <spdlog/spdlog.h>

namespace vr {

    void VmaMemoryAllocator::init() {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
        allocatorInfo.instance = instance;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;

        spdlog::debug("initializing VMA memory allocator");
        CHECK_VULKAN(vmaCreateAllocator(&allocatorInfo, &allocator));
        spdlog::info("VMA memory allocator initialized");
    }

    void VmaMemoryAllocator::destroy() {
        if(allocator) {
            spdlog::debug("destroying VMA memory allocator");
            vmaDestroyAllocator(allocator);
            spdlog::info("VMA memory allocator destroyed");
        }
    }

    Buffer VmaMemoryAllocator::allocate(VkBufferCreateInfo createInfo, VmaMemoryUsage usage) {
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = usage;

        VkBuffer buffer;
        VmaAllocation allocation;
        CHECK_VULKAN(vmaCreateBuffer(allocator, &createInfo, &allocInfo, &buffer, &allocation, nullptr));

        return { buffer, createInfo, allocation };
    }

    Image VmaMemoryAllocator::allocate(VkImageCreateInfo createInfo, VmaMemoryUsage usage) {
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = usage;

        VkImage image;
        VmaAllocation allocation;
        vmaCreateImage(allocator, &createInfo, &allocInfo, &image, &allocation, nullptr);


        return { image , createInfo, allocation };
    }


    void VmaMemoryAllocator::deallocate(Buffer buffer) {
        vmaDestroyBuffer(allocator, buffer.handle, buffer.allocation);
    }

    void VmaMemoryAllocator::deallocate(Image image) {
        vmaDestroyImage(allocator, image.handle, image.allocation);
    }


}