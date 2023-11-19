#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include <cinttypes>
#include <map>

namespace vr {

    struct Buffer {
        VkBuffer handle{};
        VkBufferCreateInfo info{};
        VmaAllocation allocation{};
    };

    struct Image {
        VkImage handle{};
        VkImageCreateInfo info{};
        VmaAllocation allocation{};
    };

    struct VmaMemoryAllocator {
        VkInstance instance{VK_NULL_HANDLE};
        VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
        VkDevice device{VK_NULL_HANDLE};
        VmaAllocator allocator{VK_NULL_HANDLE};

        void init();

        void destroy();

        Buffer allocate(VkBufferCreateInfo createInfo, VmaMemoryUsage usage);

        Image allocate(VkImageCreateInfo createInfo, VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY);

        void deallocate(Buffer buffer);

        void deallocate(Image image);
    };
}