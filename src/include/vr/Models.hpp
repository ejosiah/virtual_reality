#pragma once

#include "vr/specification/SwapchainSpecification.hpp"

#include <openxr/openxr.h>

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>

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
        ViewInfo viewInfo{};
        ImageId imageId{};
        XrSpace cameraSpace{};
    };

    struct FrameEnd {
        XrEnvironmentBlendMode blendMode{XR_ENVIRONMENT_BLEND_MODE_OPAQUE};
        std::vector<XrCompositionLayerBaseHeader*> layers;
    };

    template<typename T>
    struct Pose_type {
        glm::qua<T, glm::defaultp> orientation;
        glm::vec<3, T, glm::defaultp> position;
    };

    using Pose = Pose_type<float>;
}