#pragma once

#include <openxr/openxr.h>

#include <vector>

namespace vr {

    struct ViewInfo {
        XrViewState viewState;
        std::vector<XrView> views;
    };

    struct SwapChain {
        XrSwapchain handle{XR_NULL_HANDLE};
    };

    struct FrameInfo {

    };

    struct FrameEnd {
        XrEnvironmentBlendMode blendMode{XR_ENVIRONMENT_BLEND_MODE_OPAQUE};
        std::vector<XrCompositionLayerBaseHeader*> layers;
    };
}