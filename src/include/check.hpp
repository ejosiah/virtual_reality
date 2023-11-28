#pragma once


#include "print.hpp"

#include <openxr/openxr.h>
#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <cpptrace/cpptrace.hpp>

#include <cassert>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define FILE_AND_LINE __FILE__ ":" LINE_STRING


#define CHECK_VULKAN(result) do { if((result) < 0) { \
        fmt_error(__FILE__ "(" LINE_STRING "): VkResult( " #result " ) < 0"); \
        assert(0 && #result); \
 } } while(false)

#define CHECK_XR(result) do { if(XR_FAILED(result)) { \
        assert(XR_FAILED(result)); \
        fmt_error(__FILE__ "(" LINE_STRING "): XkResult( " #result " ) < 0"); \
 } } while(false)

#define LOG_ERROR(instance, result)  if(XR_FAILED(result)) { \
    static char buffer[XR_MAX_RESULT_STRING_SIZE]; \
    xrResultToString(instance, result, buffer); \
    spdlog::error(__FILE__   "(" LINE_STRING "):" " {}\n", buffer); \
    assert(XR_SUCCEEDED(result)); \
}

[[noreturn]] inline void Throw(std::string failureMessage, const char* originator = nullptr, const char* sourceLocation = nullptr) {
    if (originator != nullptr) {
        failureMessage += std::format("\n    Origin: {}", originator);
    }
    if (sourceLocation != nullptr) {
        failureMessage += std::format("\n    Source: {}", sourceLocation);
    }

    throw cpptrace::runtime_error(failureMessage.c_str());
}

#define THROW(msg) Throw(msg, nullptr, FILE_AND_LINE);
