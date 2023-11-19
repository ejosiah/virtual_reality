#include <iostream>
#include "vr/graphics/vulkan/VulkanContext.hpp"
#include "vr/graphics/Graphics.hpp"
#include "vr/graphics/vulkan/VulkanGraphicsService.hpp"
#include "vr/Application.hpp"

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


//    auto ctx = creation.appName("OpenXR test").create();
//    std::unique_ptr<vr::GraphicsService> graphicsService = std::make_unique<vr::VulkanGraphicsService>(ctx.value());
//    graphicsService->init();
//    graphicsService->shutdown();
//    ctx->destroy();

    auto renderer = vr::VoidRenderer::shared();
    auto graphicsFactory = vr::VulkanGraphicsService::shared;

    vr::Application application{std::move(renderer), graphicsFactory, creation };

    try {
        application.run();
    }catch(const std::runtime_error& error){
        spdlog::error("Application Error, reason: {}", error.what());
        return 500;
    }

    return 0;
}