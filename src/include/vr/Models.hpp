#pragma once

#include "vr/specification/SwapchainSpecification.hpp"

#include <openxr/openxr.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
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

    struct FrameEnd {
        XrEnvironmentBlendMode blendMode{XR_ENVIRONMENT_BLEND_MODE_OPAQUE};
        std::vector<XrCompositionLayerBaseHeader*> layers;
    };

    struct Pose {
        glm::quat orientation{1, 0, 0, 0};
        glm::vec3 position{0};
        glm::vec3 scale{1};
    };

    struct SpaceLocation {
        std::string name;
        Pose pose;
    };


    struct Cube {
        Pose pose;
    };

    struct FrameInfo {
        ImageId imageId{};
        ViewInfo viewInfo{};
        XrSpace space{};
        XrTime predictedTime{};
        XrDuration predictedDuration{};
        float aspectRatio{1};
    };


    inline Pose convert(const XrPosef& pose) {
        const auto& o = pose.orientation;
        const auto& p = pose.position;
        return {
                {o.w, o.x, o.y, o.z},
                {p.x, p.y, p.z}
        };
    }

    inline XrPosef convert(const Pose& pose) {
        const auto& o = pose.orientation;
        const auto& p = pose.position;

        return {
                {o.x, o.y, o.z, o.w},
                {p.x, p.y, p.z}
        };
    }

    inline glm::mat4 toMatrix(const Pose& pose) {
        glm::mat4 rotation = glm::mat4_cast(pose.orientation);
        glm::mat4 translation = glm::translate(glm::mat4(1), pose.position);
        glm::mat4 scale = glm::scale(glm::mat4(1), pose.scale);

        return  translation * rotation * scale;
    }

    inline glm::mat4 toMatrix(const XrPosef& pose) {
        return toMatrix(convert(pose));
    }
}