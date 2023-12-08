#pragma once

#include "Models.hpp"

#include <variant>

namespace vr {

    enum EyeVisibility : uint32_t {
        LEFT = 1u << 1,
        RIGHT = 1u << 2
    };

    struct ProjectionView {
        FieldOfView fov{};
        SubImage subImage;
        Pose pose;
    };

    struct ProjectionLayer {
        uint32_t layerId{0};
        std::vector<ProjectionView> views;
    };

    struct QuadLayer {
        uint32_t layerId{0};
        SubImage subImage;
        EyeVisibility eyeVisibility{ EyeVisibility::LEFT | EyeVisibility::RIGHT };
        Pose pose;
        glm::vec2 extent;
    };

    using CompositionLayer = std::variant<ProjectionLayer, QuadLayer>;
}