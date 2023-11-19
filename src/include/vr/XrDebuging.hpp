#pragma once

#include <openxr/openxr_platform.h>
#include <spdlog/spdlog.h>
#include <iostream>

namespace vr {

    static XrBool32 debugCallBack(            XrDebugUtilsMessageSeverityFlagsEXT              messageSeverity,
                                              XrDebugUtilsMessageTypeFlagsEXT                  messageTypes,
                                              const XrDebugUtilsMessengerCallbackDataEXT*      callbackData,
                                              void*                                            userData) {

        switch (messageSeverity) {
            case XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                spdlog::debug("{} => {}", callbackData->functionName, callbackData->message);
                break;
            case XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                spdlog::info("{} => {}", callbackData->functionName, callbackData->message);
                break;
            case XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                spdlog::warn("{} => {}", callbackData->functionName, callbackData->message);
                break;
            case XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                spdlog::error("{} => {}", callbackData->functionName, callbackData->message);
                break;
            default:
                spdlog::warn("unknown severity: {} => {}", callbackData->functionName, callbackData->message);
        }

        return XR_FALSE;
    }

    XrDebugUtilsMessengerCreateInfoEXT xrDebugCreateInfo() {
        XrDebugUtilsMessengerCreateInfoEXT createDMInfo{XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        createDMInfo.messageSeverities =
                XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createDMInfo.messageTypes =
                XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
        createDMInfo.userCallback = debugCallBack;

        return createDMInfo;
    }

}