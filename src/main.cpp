#include <iostream>
#include "vr/graphics/vulkan/VulkanContext.hpp"
#include "vr/graphics/vulkan/VulkanGraphicsService.hpp"
#include "vr/SessionConfig.hpp"
#include "vr/Application.hpp"
#include "vr/specification/Specifications.hpp"
#include "CheckerboardRenderer.hpp"
#include "ClearScreen.hpp"
#include "SpaceVisiualization.hpp"

#include <vulkan/vulkan.h>

int main() {
    spdlog::set_level(spdlog::level::debug);

    auto renderer = ClearScreen::shared();
    auto creation = vr::ContextCreation::vulkan();

    creation
        .appName(renderer->name())
        .formFactor(XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY)
            .graphics<vr::VulkanContextCreation>()
                .appName(renderer->name())
                .swapChainImageFormat(VK_FORMAT_R8G8B8A8_SRGB)
                .apiVersion(VK_API_VERSION_1_3)
                .addLayer("VK_LAYER_KHRONOS_validation");


    vr::SessionConfig sessionConfig = ClearScreen::session();

    vr::Application application{
        creation
        , sessionConfig
        , renderer
        , vr::VulkanGraphicsService::shared };

    try {
        application.run();
    }catch(const std::runtime_error& error){
        spdlog::error("Application Error, reason: {}", error.what());
        return 500;
    }

    return 0;
}