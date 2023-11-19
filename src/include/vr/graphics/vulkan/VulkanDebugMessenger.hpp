#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

namespace vr {

    static VkBool32 VKAPI_PTR debugCallBack(
            VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
            void*                                            pUserData){

        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT){
            spdlog::debug(pCallbackData->pMessage);
        }

        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT){
            spdlog::info(pCallbackData->pMessage);
        }
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
            spdlog::warn(pCallbackData->pMessage);
        }
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT){
            spdlog::error(pCallbackData->pMessage);
        }

        return VK_FALSE;
    }


    static VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo(){
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

        createInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        createInfo.pfnUserCallback = debugCallBack;

        return createInfo;
    }

}