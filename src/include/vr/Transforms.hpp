#pragma once

#include <openxr/openxr.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>


namespace vr {

    struct Pose {
        glm::quat orientation{1, 0, 0, 0};
        glm::vec3 position{0};
    };

    struct SpaceLocation {
        std::string name;
        Pose pose;
    };

    struct Transform {
        Pose pose{};
        glm::vec3 scale{1};

        explicit operator glm::mat4() const {
            glm::mat4 translation = glm::translate(glm::mat4(1), pose.position);
            glm::mat4 rotation = glm::mat4_cast(pose.orientation);
            glm::mat4 aScale = glm::scale(glm::mat4(1), scale);

            return translation * rotation * aScale;
        }
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

        return  translation * rotation;
    }

    inline glm::mat4 toMatrix(const XrPosef& pose) {
        return toMatrix(convert(pose));
    }
}