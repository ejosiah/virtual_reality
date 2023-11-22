#pragma once

#include "check.hpp"

#include <vulkan/vulkan.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <stdexcept>
#include <format>
#include <type_traits>

template<typename T>
inline T makeStruct() {
    THROW(std::format("makeStruct not implemented for type: {}", typeid(T).name()));
}

template<>
inline XrApiLayerProperties makeStruct<XrApiLayerProperties>() {
    return { XR_TYPE_API_LAYER_PROPERTIES };
}

template<>
inline XrInstanceCreateInfo makeStruct<XrInstanceCreateInfo>() {
    return { XR_TYPE_INSTANCE_CREATE_INFO };
}

template<>
inline XrInstanceProperties makeStruct<XrInstanceProperties>() {
    return { XR_TYPE_INSTANCE_PROPERTIES };
}

template<>
inline XrSystemGetInfo makeStruct<XrSystemGetInfo>() {
    return { XR_TYPE_SYSTEM_GET_INFO };
}

template<>
inline XrSystemProperties makeStruct<XrSystemProperties>() {
    return { XR_TYPE_SYSTEM_PROPERTIES};
}


template<>
inline XrViewConfigurationProperties makeStruct<XrViewConfigurationProperties>() {
    return { XR_TYPE_VIEW_CONFIGURATION_PROPERTIES };
}

template<>
inline XrViewConfigurationView makeStruct<XrViewConfigurationView>() {
    return { XR_TYPE_VIEW_CONFIGURATION_VIEW };
}

template<>
inline XrGraphicsRequirementsVulkanKHR makeStruct<XrGraphicsRequirementsVulkanKHR>() {
    return { XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
}

template<>
inline XrVulkanInstanceCreateInfoKHR makeStruct<XrVulkanInstanceCreateInfoKHR>() {
    return { XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR };
}

template<>
inline XrSessionCreateInfo makeStruct<XrSessionCreateInfo>() {
    return { XR_TYPE_SESSION_CREATE_INFO };
}

template<>
inline XrSessionBeginInfo makeStruct<XrSessionBeginInfo>() {
    return { XR_TYPE_SESSION_BEGIN_INFO };
}

template<>
inline XrGraphicsBindingVulkanKHR makeStruct<XrGraphicsBindingVulkanKHR>() {
    return { XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR };
}

template<>
inline XrVulkanGraphicsDeviceGetInfoKHR makeStruct<XrVulkanGraphicsDeviceGetInfoKHR>() {
    return { XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR };
}

template<>
inline XrSwapchainCreateInfo makeStruct<XrSwapchainCreateInfo>() {
    return { XR_TYPE_SWAPCHAIN_CREATE_INFO };
}

template<>
inline XrSwapchainImageBaseHeader makeStruct<XrSwapchainImageBaseHeader>() {
    return { XR_TYPE_UNKNOWN };
}


template<>
inline XrSwapchainImageVulkanKHR makeStruct<XrSwapchainImageVulkanKHR>() {
    return { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR };
}

template<>
inline XrVulkanDeviceCreateInfoKHR makeStruct<XrVulkanDeviceCreateInfoKHR>() {
    return { XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR };
}

template<>
inline XrFrameWaitInfo makeStruct<XrFrameWaitInfo>() {
    return { XR_TYPE_FRAME_WAIT_INFO };
}

template<>
inline XrFrameState makeStruct<XrFrameState>() {
    return { XR_TYPE_FRAME_STATE };
}

template<>
inline XrSwapchainImageAcquireInfo makeStruct<XrSwapchainImageAcquireInfo>() {
    return { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
}

template<>
inline XrSwapchainImageWaitInfo makeStruct<XrSwapchainImageWaitInfo>() {
    return { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
}

template<>
inline XrSwapchainImageReleaseInfo makeStruct<XrSwapchainImageReleaseInfo>() {
    return { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
}

template<>
inline XrFrameEndInfo makeStruct<XrFrameEndInfo>() {
    return { XR_TYPE_FRAME_END_INFO };
}

template<>
inline XrReferenceSpaceCreateInfo makeStruct<XrReferenceSpaceCreateInfo>() {
    return { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
}

template<>
inline XrViewLocateInfo makeStruct<XrViewLocateInfo>() {
    return { XR_TYPE_VIEW_LOCATE_INFO };
}

template<>
inline XrViewState makeStruct<XrViewState>() {
    return { XR_TYPE_VIEW_STATE };
}

template<>
inline XrView makeStruct<XrView>() {
    return { XR_TYPE_VIEW };
}

template<>
inline XrEventDataBuffer makeStruct<XrEventDataBuffer>() {
    return { XR_TYPE_EVENT_DATA_BUFFER };
}

template<>
inline XrCompositionLayerProjection makeStruct<XrCompositionLayerProjection>() {
    return { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
}

template<>
inline XrCompositionLayerProjectionView makeStruct<XrCompositionLayerProjectionView>() {
    return { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
}

/***********************************************************************
         __   __    _ _               ___ _               _
         \ \ / /  _| | |____ _ _ _   / __| |_ _ _ _  _ __| |_ ___
          \ V / || | | / / _` | ' \  \__ \  _| '_| || / _|  _(_-<
           \_/ \_,_|_|_\_\__,_|_||_| |___/\__|_|  \_,_\__|\__/__/

***********************************************************************/



template<>
inline VkBufferCreateInfo makeStruct<VkBufferCreateInfo>() {
    return { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
}

template<>
inline VkImageCreateInfo makeStruct<VkImageCreateInfo>() {
    return { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
}

template<>
inline VkCommandBufferAllocateInfo makeStruct<VkCommandBufferAllocateInfo>() {
    return { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
}

template<>
inline VkCommandBufferBeginInfo makeStruct<VkCommandBufferBeginInfo>() {
    return { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
}

template<>
inline VkSubmitInfo makeStruct<VkSubmitInfo>() {
    return { VK_STRUCTURE_TYPE_SUBMIT_INFO };
}

template<>
inline VkBufferMemoryBarrier makeStruct<VkBufferMemoryBarrier>() {
    return { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
}

template<>
inline VkMemoryBarrier makeStruct<VkMemoryBarrier>() {
    return { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
}

template<>
inline VkImageMemoryBarrier makeStruct<VkImageMemoryBarrier>() {
    return { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
}

template<>
inline VkRenderingInfo makeStruct<VkRenderingInfo>() {
    return { VK_STRUCTURE_TYPE_RENDERING_INFO };
}

template<>
inline VkRenderingAttachmentInfo makeStruct<VkRenderingAttachmentInfo>() {
    return { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
}

template<>
inline VkImageViewCreateInfo makeStruct<VkImageViewCreateInfo>() {
    return { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
}

template<>
inline VkPhysicalDeviceDynamicRenderingFeatures makeStruct<VkPhysicalDeviceDynamicRenderingFeatures>() {
    return { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES };
}

template<>
inline VkDeviceCreateInfo makeStruct<VkDeviceCreateInfo>() {
    return { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
}