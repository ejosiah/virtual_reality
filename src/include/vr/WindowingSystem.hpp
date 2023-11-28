#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <cinttypes>

namespace vr {

    struct Window {
        std::string title{};
        uint32_t width{};
        uint32_t height{};
        GLFWwindow* _{};
    };

    class WindowingSystem {
    public:
        static void init();

        static std::vector<const char *> vulkanExtensions();

        static Window createWindow(const std::string& title, uint32_t width, uint32_t height);

        static void destroy(Window& window);

        static VkSurfaceKHR surface(const Window& window, VkInstance instance);

        static void shutdown();

    private:


    };

}