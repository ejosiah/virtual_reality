#pragma once

#include "Models.hpp"


#include <glm/glm.hpp>

#include <string>
#include <variant>
#include <map>

namespace vr {

    using ActionState = std::variant<bool, float, glm::vec2, Pose>;

    struct Action {
        std::string name;
        bool changed{};
        bool isActive{};
        ActionState _value;

        template<typename T>
        T value() const {
            return std::get<T>(_value);
        }

        auto operator<=>(const Action& other) const {
            return name<=>other.name;
        }
    };

    struct ActionSet {
        std::string name;
        std::map<std::string, Action> actions;

        auto begin() {
            return actions.begin();
        }

        auto end() {
            return actions.end();
        }

        [[nodiscard]]
        auto cbegin() const {
            return actions.cbegin();
        }

        [[nodiscard]]
        auto cend() const {
            return actions.cend();
        }

        [[nodiscard]]
        const Action& get(const std::string& name) const {
            return actions.at(name);
        }
    };

    struct Vibrate {
        std::string action;
        XrDuration duration{XR_MIN_HAPTIC_DURATION};
        float frequency{XR_FREQUENCY_UNSPECIFIED};
        float amplitude{0.5};
    };
}