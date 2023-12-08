#pragma once

#include <vulkan/vulkan.h>
#include <openxr/openxr.h>

struct XrVulkanSwapChain{
    std::string name{};
    XrSwapchain _{};
    uint32_t width{};
    uint32_t height{};
    VkFormat format;
    std::vector<XrSwapchainImageVulkanKHR> images{};

    auto begin() noexcept {
        return images.begin();
    }

    auto end() noexcept {
        return images.end();
    }

    auto cbegin() const noexcept {
        return images.begin();
    }

    auto cend() const noexcept {
        return images.end();
    }

};