#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "vr/graphics/vulkan/VulkanContext.hpp"
#include "vr/graphics/vulkan/VulkanGraphicsService.hpp"
#include "vr/SessionConfig.hpp"
#include "vr/Application.hpp"
#include "vr/specification/Specifications.hpp"
#include "CheckerboardRenderer.hpp"
#include "ClearScreen.hpp"
#include "vr/WindowingSystem.hpp"

#include "SpaceVisiualization.hpp"
#include "EnvironmentRenderer.hpp"
#include <iostream>

int main() {
    spdlog::set_level(spdlog::level::debug);

    auto renderer = EnvironmentRenderer::shared();
    auto creation = vr::ContextCreation::vulkan();

    creation
        .appName(renderer->name())
        .formFactor(XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY)
            .graphics<vr::VulkanContextCreation>()
                .appName(renderer->name())
                .apiVersion(VK_API_VERSION_1_3)
                .addLayer("VK_LAYER_KHRONOS_validation");


    vr::SessionConfig sessionConfig = EnvironmentRenderer::session();

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
