#pragma once

#include "Transforms.hpp"
#include "vr/specification/SwapchainSpecification.hpp"

#include <openxr/openxr.h>

#include <vector>
#include <string>

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
        std::string swapChain;
        uint32_t imageIndex{};
    };

    struct FrameInfo {
        ImageId imageId{};
        ViewInfo viewInfo{};
        XrSpace space{};
        XrTime predictedTime{};
        XrDuration predictedDuration{};
    };


    struct FrameEnd {
        XrEnvironmentBlendMode blendMode{XR_ENVIRONMENT_BLEND_MODE_OPAQUE};
        std::vector<XrCompositionLayerBaseHeader*> layers;
    };

}