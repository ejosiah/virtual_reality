#pragma once

#include "Transforms.hpp"
#include "vr/specification/SwapchainSpecification.hpp"

#include <vulkan/vulkan.h>
#include <openxr/openxr.h>

#include <vector>
#include <string>
#include <span>
#include <variant>
#include <optional>

namespace vr {

    struct ViewInfo {
        XrViewState viewState;
        std::vector<XrView> views;
    };

    struct SwapChain {
        SwapchainSpecification spec;
        XrSwapchain handle{XR_NULL_HANDLE};
    };

    struct ImageId {
        XrSwapchain swapChain;
        uint32_t imageIndex{};
    };

    struct SubImage {
        std::string swapChain;
        glm::vec2 region;
        uint32_t arrayIndex;
    };

    struct FieldOfView {
        float angleLeft;
        float angleRight;
        float angleUp;
        float angleDown;
    };

    struct FrameInfo {
        ImageId imageId{};
        ViewInfo viewInfo{};
        XrSpace space{};
        XrTime predictedTime{};
        XrDuration predictedDuration{};
        std::optional<ImageId> depthImageId;
    };

    using CompositionLayer =
        std::variant<
            XrCompositionLayerProjection*,
            XrCompositionLayerQuad*,
            XrCompositionLayerCubeKHR*,
            XrCompositionLayerEquirectKHR*,
            XrCompositionLayerCylinderKHR*,
            XrCompositionLayerBaseHeader*
        >;

    struct Layer {
        CompositionLayer layer;
        uint32_t position{};

        auto operator<=>(const Layer& other) const {
            return position<=>other.position;
        }
    };

    using Layers = std::vector<Layer>;

}