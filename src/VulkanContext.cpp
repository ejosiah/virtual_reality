#include "vr/graphics/vulkan/VulkanContext.hpp"

#ifndef NDEBUG
#ifndef XR_DEBUG
#ifdef VK_DEBUG
#include "vr/graphics/vulkan/VulkanDebugMessenger.hpp"
#endif
#endif

#endif
#include <vulkan_ext_loader.h>
#include <openxr_ext_loader.h>
#include <spdlog/spdlog.h>
#include <format>


namespace vr {

    void validateVersions(const Context &ctx, uint32_t version) {
        auto graphicsRequirement = makeStruct<XrGraphicsRequirementsVulkanKHR>();
        xrGetVulkanGraphicsRequirements2KHR(ctx.instance, ctx.systemId, &graphicsRequirement);

        if(VK_VERSION_MAJOR(version) < XR_VERSION_MAJOR(graphicsRequirement.minApiVersionSupported)
           || VK_VERSION_MINOR(version) < XR_VERSION_MINOR(graphicsRequirement.minApiVersionSupported)) {

            auto minVersion = std::format("{}.{}.{}"
                    , XR_VERSION_MAJOR(graphicsRequirement.minApiVersionSupported)
                    , XR_VERSION_MINOR(graphicsRequirement.minApiVersionSupported)
                    , XR_VERSION_PATCH(graphicsRequirement.minApiVersionSupported));
            throw std::runtime_error{std::format("VulkanContextCreation: minimum required version {}", minVersion)};
        }
    }

    void VulkanContextCreation::validate(const Context &ctx) const {
        if(_appName.empty()) {
            throw std::runtime_error{"VulkanContextCreation: appName required"};
        }
        if(_apiVersion == 0) {
            throw std::runtime_error{"VulkanContextCreation: apiVersion required"};
        }
        if(_format == VK_FORMAT_UNDEFINED) {
            throw std::runtime_error{"VulkanContextCreation: swapChainImageFormat required"};
        }

        validateVersions(ctx, _apiVersion);

    }

    std::shared_ptr<GraphicsContext> VulkanContextCreation::create(const Context &ctx) const {
        validate(ctx);
        VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.applicationVersion = _appVersion;
        appInfo.pApplicationName = _appName.c_str();
        appInfo.apiVersion = _apiVersion;
        appInfo.pEngineName = _engineName.c_str();
        appInfo.engineVersion = _engineVersion;

        VkInstanceCreateInfo vulkanCreatInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

#ifndef NDEBUG
#ifndef XR_DEBUG
#ifdef VK_DEBUG
        auto createDebugInfo = vr::debugCreateInfo();
        vulkanCreatInfo.pNext = &createDebugInfo;
#endif
#endif
#endif
        vulkanCreatInfo.pApplicationInfo = &appInfo;
        vulkanCreatInfo.enabledExtensionCount = extensions.size();
        vulkanCreatInfo.ppEnabledExtensionNames = extensions.data();

        auto xrVulkanCreateInfo = makeStruct<XrVulkanInstanceCreateInfoKHR>();
        xrVulkanCreateInfo.systemId = ctx.systemId;
        xrVulkanCreateInfo.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        xrVulkanCreateInfo.vulkanCreateInfo = &vulkanCreatInfo;

        VkInstance instance;
        VkResult result;
        LOG_ERROR(ctx.instance, xrCreateVulkanInstanceKHR(ctx.instance, &xrVulkanCreateInfo, &instance, &result))
        init_vulkan_ext(instance);


        auto vulkanCtx = std::make_shared<VulkanContext>();
        vulkanCtx->instance = instance;
        vulkanCtx->apiVersion = _apiVersion;
        vulkanCtx->swapChainImageFormat = _format;
#ifndef NDEBUG
#ifndef XR_DEBUG
#ifdef VK_DEBUG
        vkCreateDebugUtilsMessengerEXT(instance, &createDebugInfo, VK_NULL_HANDLE, &vulkanCtx->debugMessenger);
#endif
#endif
#endif

        return vulkanCtx;
    }

}