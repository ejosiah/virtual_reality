#include "openxr_ext_loader.h"
#include <string>
#include <tuple>

XrInstance g_instance;

template<typename Proc>
std::tuple<XrResult, Proc> instanceProc(const char*  name) {
    Proc proc = nullptr;
    auto result = xrGetInstanceProcAddr(g_instance, name, reinterpret_cast<PFN_xrVoidFunction*>(&proc));
    return std::make_tuple(result, proc);
}

#ifndef NDEBUG
XrResult XRAPI_PTR xrCreateDebugUtilsMessengerEXT(XrInstance instance, const XrDebugUtilsMessengerCreateInfoEXT* createInfo, XrDebugUtilsMessengerEXT* messenger) {
    static auto [result, proc] = instanceProc<PFN_xrCreateDebugUtilsMessengerEXT>("xrCreateDebugUtilsMessengerEXT");
    return XR_SUCCEEDED(result) ? proc(instance, createInfo, messenger) : result;
}

XrResult XRAPI_PTR xrSetDebugUtilsObjectNameEXT(XrInstance instance, const XrDebugUtilsObjectNameInfoEXT* nameInfo) {
    static auto [result, proc] = instanceProc<PFN_xrSetDebugUtilsObjectNameEXT>("xrSetDebugUtilsObjectNameEXT");
    return XR_SUCCEEDED(result) ? proc(instance, nameInfo) : result;

}

XrResult XRAPI_PTR xrDestroyDebugUtilsMessengerEXT(XrDebugUtilsMessengerEXT messenger) {
    static auto [result, proc] = instanceProc<PFN_xrDestroyDebugUtilsMessengerEXT>("xrDestroyDebugUtilsMessengerEXT");
    return XR_SUCCEEDED(result) ? proc(messenger) : result;
}

XrResult XRAPI_PTR xrSubmitDebugUtilsMessageEXT(XrInstance  instance,
                                                XrDebugUtilsMessageSeverityFlagsEXT  messageSeverity,
                                                XrDebugUtilsMessageTypeFlagsEXT  messageTypes,
                                                const XrDebugUtilsMessengerCallbackDataEXT* callbackData) {
    static auto [result, proc] = instanceProc<PFN_xrSubmitDebugUtilsMessageEXT>("xrSubmitDebugUtilsMessageEXT");
    return XR_SUCCEEDED(result) ? proc(instance, messageSeverity, messageTypes, callbackData) : result;
}

XrResult XRAPI_PTR xrSessionBeginDebugUtilsLabelRegionEXT(XrSession session, const XrDebugUtilsLabelEXT* labelInfo) {
    static auto [result, proc] = instanceProc<PFN_xrSessionBeginDebugUtilsLabelRegionEXT>("xrSessionBeginDebugUtilsLabelRegionEXT");
    return XR_SUCCEEDED(result) ? proc(session, labelInfo) : result;

}

XrResult XRAPI_PTR xrSessionEndDebugUtilsLabelRegionEXT(XrSession session) {
    static auto [result, proc] = instanceProc<PFN_xrSessionEndDebugUtilsLabelRegionEXT>("xrSessionEndDebugUtilsLabelRegionEXT");
    return XR_SUCCEEDED(result) ? proc(session) : result;

}

XrResult XRAPI_PTR xrSessionInsertDebugUtilsLabelEXT(XrSession session, const XrDebugUtilsLabelEXT* labelInfo) {
    static auto [result, proc] = instanceProc<PFN_xrSessionInsertDebugUtilsLabelEXT>("xrSessionInsertDebugUtilsLabelEXT");
    return XR_SUCCEEDED(result) ? proc(session, labelInfo) : result;

}
#endif

XrResult  xrGetVulkanGraphicsRequirements2KHR(
        XrInstance                                  instance,
        XrSystemId                                  systemId,
        XrGraphicsRequirementsVulkanKHR*            graphicsRequirements) {
    static auto [result, proc] = instanceProc<PFN_xrGetVulkanGraphicsRequirements2KHR>("xrGetVulkanGraphicsRequirements2KHR");
    return XR_SUCCEEDED(result) ? proc(instance, systemId, graphicsRequirements) : result;
}

XrResult xrCreateVulkanInstanceKHR(XrInstance instance, const XrVulkanInstanceCreateInfoKHR *createInfo,
                                   VkInstance *vulkanInstance, VkResult *vulkanResult) {
    static auto [result, proc] = instanceProc<PFN_xrCreateVulkanInstanceKHR>("xrCreateVulkanInstanceKHR");
    return XR_SUCCEEDED(result) ? proc(instance, createInfo, vulkanInstance, vulkanResult) : result;}

XrResult xrCreateVulkanDeviceKHR(XrInstance instance,
                                 const XrVulkanDeviceCreateInfoKHR *createInfo,
                                 VkDevice *vulkanDevice,
                                 VkResult *vulkanResult) {
    static auto [result, proc] = instanceProc<PFN_xrCreateVulkanDeviceKHR>("xrCreateVulkanDeviceKHR");
    return XR_SUCCEEDED(result) ? proc(instance, createInfo, vulkanDevice, vulkanResult) : result;
}

XrResult xrGetVulkanGraphicsDevice2KHR(XrInstance instance,
                                       const XrVulkanGraphicsDeviceGetInfoKHR *getInfo,
                                       VkPhysicalDevice *vulkanPhysicalDevice) {
    static auto [result, proc] = instanceProc<PFN_xrGetVulkanGraphicsDevice2KHR>("xrGetVulkanGraphicsDevice2KHR");
    return XR_SUCCEEDED(result) ? proc(instance, getInfo, vulkanPhysicalDevice) : result;
}

XrResult xrGetVulkanInstanceExtensionsKHR(XrInstance instance, XrSystemId systemId, uint32_t bufferCapacityInput,
                                          uint32_t *bufferCountOutput, char *buffer) {
    static auto [result, proc] = instanceProc<PFN_xrGetVulkanInstanceExtensionsKHR>("xrGetVulkanInstanceExtensionsKHR");
    return XR_SUCCEEDED(result) ? proc(instance, systemId, bufferCapacityInput, bufferCountOutput, buffer) : result;
}

XrResult xrGetVulkanDeviceExtensionsKHR(XrInstance instance, XrSystemId systemId, uint32_t bufferCapacityInput,
                                        uint32_t *bufferCountOutput, char *buffer) {
    static auto [result, proc] = instanceProc<PFN_xrGetVulkanDeviceExtensionsKHR>("xrGetVulkanDeviceExtensionsKHR");
    return XR_SUCCEEDED(result) ? proc(instance, systemId, bufferCapacityInput, bufferCountOutput, buffer) : result;
}
