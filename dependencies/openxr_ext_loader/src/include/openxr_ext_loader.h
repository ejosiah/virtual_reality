#pragma once

#include <vulkan/vulkan.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

extern XrInstance g_instance;

inline void initExtLoader(XrInstance instance) {
    g_instance = instance;
}

XrResult XRAPI_PTR xrCreateDebugUtilsMessengerEXT(XrInstance instance, const XrDebugUtilsMessengerCreateInfoEXT* createInfo, XrDebugUtilsMessengerEXT* messenger);

XrResult XRAPI_PTR xrSetDebugUtilsObjectNameEXT(XrInstance instance, const XrDebugUtilsObjectNameInfoEXT* nameInfo);

XrResult XRAPI_PTR xrDestroyDebugUtilsMessengerEXT(XrDebugUtilsMessengerEXT messenger);

XrResult XRAPI_PTR xrSubmitDebugUtilsMessageEXT(XrInstance  instance, XrDebugUtilsMessageSeverityFlagsEXT  messageSeverity, XrDebugUtilsMessageTypeFlagsEXT  messageTypes, const XrDebugUtilsMessengerCallbackDataEXT* callbackData);

XrResult XRAPI_PTR xrSessionBeginDebugUtilsLabelRegionEXT(XrSession session, const XrDebugUtilsLabelEXT* labelInfo);

XrResult XRAPI_PTR xrSessionEndDebugUtilsLabelRegionEXT(XrSession session);

XrResult XRAPI_PTR xrSessionInsertDebugUtilsLabelEXT(XrSession session, const XrDebugUtilsLabelEXT* labelInfo);

XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanInstanceExtensionsKHR(
        XrInstance                                  instance,
        XrSystemId                                  systemId,
        uint32_t                                    bufferCapacityInput,
        uint32_t*                                   bufferCountOutput,
        char*                                       buffer);

XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanDeviceExtensionsKHR(
        XrInstance                                  instance,
        XrSystemId                                  systemId,
        uint32_t                                    bufferCapacityInput,
        uint32_t*                                   bufferCountOutput,
        char*                                       buffer);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateVulkanInstanceKHR(
        XrInstance                                  instance,
        const XrVulkanInstanceCreateInfoKHR*        createInfo,
        VkInstance*                                 vulkanInstance,
        VkResult*                                   vulkanResult);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateVulkanDeviceKHR(
        XrInstance                                  instance,
        const XrVulkanDeviceCreateInfoKHR*          createInfo,
        VkDevice*                                   vulkanDevice,
        VkResult*                                   vulkanResult);

XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanGraphicsDevice2KHR(
        XrInstance                                  instance,
        const XrVulkanGraphicsDeviceGetInfoKHR*     getInfo,
        VkPhysicalDevice*                           vulkanPhysicalDevice);

XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanGraphicsRequirements2KHR(
        XrInstance                                  instance,
        XrSystemId                                  systemId,
        XrGraphicsRequirementsVulkanKHR*            graphicsRequirements);