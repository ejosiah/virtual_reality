#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include <cinttypes>
#include <map>

namespace vr {

    struct Buffer {
        VkBuffer _{};
        VkBufferCreateInfo info{};
        VmaAllocation allocation{};
    };

    struct Image {
        VkImage handle{};
        VkImageCreateInfo info{};
        VmaAllocation allocation{};
    };

    struct Mapping {
        friend class VulkanGraphicsService;
        void* _{};

        template<typename T>
        T* as() {
            return reinterpret_cast<T*>(_);
        }

        void unmap() {
            if(_) {
                assert(allocation != nullptr && allocator != nullptr);
                vmaUnmapMemory(allocator, allocation);
                _ = nullptr;
            }
        }
    private:
        VmaAllocation allocation{};
        VmaAllocator allocator{};

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