#include <iostream>
#include "vr/graphics/vulkan/VulkanContext.hpp"
#include "vr/graphics/Graphics.hpp"
#include "vr/graphics/vulkan/VulkanGraphicsService.hpp"
#include "vr/SessionConfig.hpp"
#include "vr/Application.hpp"
#include "vr/specification/Specifications.hpp"

#include <vulkan/vulkan.h>

int main() {
    spdlog::set_level(spdlog::level::debug);
    auto creation = vr::ContextCreation::vulkan();

    creation
        .appName("OpenXR test")
        .formFactor(XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY)
            .graphics<vr::VulkanContextCreation>()
                .appName("OpenXR test")
                .swapChainImageFormat(VK_FORMAT_R8G8B8A8_UNORM)
                .apiVersion(VK_API_VERSION_1_3);


    vr::SessionConfig sessionConfig{};

    sessionConfig.addSwapChain(
        vr::SwapchainSpecification()
            .usage()
                .colorAttachment()
            .format(VK_FORMAT_R8G8B8A8_UNORM)
            .width(2048)
            .height(2048));


    vr::Application application{
        creation
        , sessionConfig
        , vr::VoidRenderer::shared()
        , vr::VulkanGraphicsService::shared };

    try {
        application.run();
    }catch(const std::runtime_error& error){
        spdlog::error("Application Error, reason: {}", error.what());
        return 500;
    }

    return 0;
}