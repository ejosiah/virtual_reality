#include "check.hpp"
#include "vr/WindowingSystem.hpp"

#include <spdlog/spdlog.h>

namespace vr {

    void WindowingSystem::init() {
        if(!glfwInit()){
            THROW("Failed to initialize GLFW")
        }
        if(!glfwVulkanSupported()) {
            THROW("Vulkan not supported");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        spdlog::info("Windowing system initialized");
    }


    std::vector<const char *> WindowingSystem::vulkanExtensions() {
        uint32_t count;
        auto cExt = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char*> extenisions(cExt, cExt + count);
        return extenisions;
    }

    Window WindowingSystem::createWindow(const std::string& title, uint32_t width, uint32_t height) {
        return Window{ title, width, height
                       , glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.c_str(), nullptr, nullptr) };
    }

    VkSurfaceKHR WindowingSystem::surface(const Window& window, VkInstance instance) {
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(instance, window._, nullptr, &surface);
        return surface;
    }

    void WindowingSystem::shutdown() {
        glfwTerminate();
        spdlog::info("Windowing system terminated");
    }

    void WindowingSystem::destroy(Window &window) {
        glfwDestroyWindow(window._);
        window._ = nullptr;
    }

    void WindowingSystem::pollEvents() {
        glfwPollEvents();
    }
}