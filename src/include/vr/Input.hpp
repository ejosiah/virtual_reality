#pragma once

#include "check.hpp"
#include <openxr/openxr.h>

#include <tuple>
#include <string>
#include <type_traits>
#include <format>

namespace vr {

    inline namespace input {

        enum class Source {
            HAND_LEFT,
            HAND_RIGHT,
            HEAD,
            GAME_PAD,
            TREADMILL
        };


        enum class Identifier {
            MENU,
            SELECT,
            TRACK_PAD,
            THUMB_STICK,
            JOY_STICK,
            TRIGGER,
            THROTTLE,
            TRACKBALL,
            PEDAL,
            SYSTEM,
            D_PAD_UP,
            D_PAD_DOWN,
            D_PAD_LEFT,
            D_PAD_RIGHT,
            A,
            B,
            X,
            Y,
            VOLUME_UP,
            VOLUME_DOWN,
            THUMB_REST,
            SHOULDER,
            SQUEEZE,
            WHEEL,
            GRIP,
            AIM,
            HAPTIC
        };

        enum class Component {
            CLICK,
            TOUCH,
            FORCE,
            VALUE,
            X,
            Y,
            TWIST,
            POSE,
            VIBRATE
        };

        struct Input {
            Source source;
            Identifier identifier;
            Component component;
        };

        class SimpleInteractionProfile;

        class OculusTouchControllerProfile;

        class InteractionProfile {
        public:
            virtual std::string path(Identifier identifier) = 0;

            virtual std::string profile() const = 0;

            virtual std::string path(const Input &input) {
                return std::format("{}{}{}", path(input.source), path(input.identifier), path(input.component));
            }

            [[nodiscard]]
            static std::string path(Component component) {
                switch (component) {
                    case Component::VIBRATE:
                        return "";
                    case Component::CLICK:
                        return "click";
                    case Component::TOUCH:
                        return "touch";
                    case Component::FORCE:
                        return "force";
                    case Component::VALUE:
                        return "value";
                    case Component::X:
                        return "x";
                    case Component::Y:
                        return "y";
                    case Component::TWIST:
                        return "twist";
                    case Component::POSE:
                        return "pose";
                }
            }

            [[nodiscard]]
            static std::string path(Source source) {
                switch (source) {
                    case Source::HAND_LEFT:
                        return "/user/hand/left";
                    case Source::HAND_RIGHT:
                        return "/user/hand/right";
                    case Source::HEAD:
                        return "/user/head";
                    case Source::GAME_PAD:
                        return "/user/gamepad";
                    case Source::TREADMILL:
                        return "/user/treadmill";
                }
            }

            static SimpleInteractionProfile Simple();

            static OculusTouchControllerProfile OculusTouchController();

        protected:
            InteractionProfile() = default;

        };

        class SimpleInteractionProfile final : public InteractionProfile {
        public:

            [[nodiscard]]
            std::string profile() const final {
                return "/interaction_profiles/khr/simple_controller";
            }

            std::string path(Identifier identifier) final {
                switch (identifier) {
                    case Identifier::SELECT:
                        return "input/select";
                    case Identifier::MENU:
                        return "input/menu";
                    case Identifier::GRIP:
                        return "input/grip";
                    case Identifier::AIM:
                        return "input/aim";
                    case Identifier::HAPTIC:
                        return "output/haptic";
                    default:
                        THROW(std::format("identifier supported for profile: {}", profile()));
                }
            }
        };

        class OculusTouchControllerProfile final : public InteractionProfile {
        public:
            [[nodiscard]]
            std::string profile() const final {
                return "/interaction_profiles/oculus/touch_controller";
            }

            std::string path(const Input &input) final {
                Input aInput = input;
                if (input.source == Source::HAND_LEFT) {
                    switch (input.identifier) {
                        case Identifier::A:
                            aInput.identifier = Identifier::X;
                            break;
                        case Identifier::B:
                            aInput.identifier = Identifier::Y;
                        default:
                            break;
                    }
                }
                if (input.source == Source::HAND_RIGHT) {
                    switch (input.identifier) {
                        case Identifier::X:
                            aInput.identifier = Identifier::A;
                            break;
                        case Identifier::Y:
                            aInput.identifier = Identifier::B;
                        case Identifier::MENU:
                            THROW("identifier not supported for right hand")
                        default:
                            break;
                    }
                }
                return InteractionProfile::path(aInput);
            }

            std::string path(Identifier identifier) final {
                switch (identifier) {
                    case Identifier::X:
                        return "input/x";
                    case Identifier::Y:
                        return "input/y";
                    case Identifier::A:
                        return "input/a";
                    case Identifier::B:
                        return "input/b";
                    case Identifier::MENU:
                        return "input/menu";
                    case Identifier::SQUEEZE:
                        return "input/squeeze";
                    case Identifier::TRIGGER:
                        return "input/trigger";
                    case Identifier::THUMB_STICK:
                        return "input/thumbstick";
                    case Identifier::THUMB_REST:
                        return "input/thumbrest";
                    case Identifier::GRIP:
                        return "input/grip";
                    case Identifier::AIM:
                        return "input/aim";
                    case Identifier::HAPTIC:
                        return "output/haptic";
                    default:
                        THROW(std::format("identifier supported for profile: {}", profile()));
                }
            }
        };

        SimpleInteractionProfile InteractionProfile::Simple() {
            SimpleInteractionProfile{};
        }

        OculusTouchControllerProfile InteractionProfile::OculusTouchController() {
            OculusTouchControllerProfile{};
        }

        inline XrActionType getActionType(Component component) {
            switch(component){
                case Component::CLICK:
                case Component::TOUCH:
                    return XR_ACTION_TYPE_BOOLEAN_INPUT;
                case Component::FORCE:
                case Component::VALUE:
                case Component::TWIST:
                    return XR_ACTION_TYPE_FLOAT_INPUT;
                case Component::Y:
                case Component::X:
                    return XR_ACTION_TYPE_VECTOR2F_INPUT;
                case Component::POSE:
                    return  XR_ACTION_TYPE_POSE_INPUT;
                case Component::VIBRATE:
                    return XR_ACTION_TYPE_VIBRATION_OUTPUT;
            }
        }

    }
}