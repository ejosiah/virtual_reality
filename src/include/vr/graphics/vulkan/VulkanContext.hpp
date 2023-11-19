#pragma once

#include "vr/forwards.hpp"
#include "vr/Context.hpp"
#include "vr/Creation.hpp"
#include <vulkan/vulkan.h>
#include <openxr/openxr.h>

#include <openxr/openxr_platform.h>
#include <vector>


namespace vr {
    struct VulkanContext : GraphicsContext {
        VkInstance instance = VK_NULL_HANDLE;
        uint32_t apiVersion = 0;
        VkFormat swapChainImageFormat{VK_FORMAT_UNDEFINED};



#ifndef NDEBUG
#ifdef VK_DEBUG
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
#endif
#endif
    };


}