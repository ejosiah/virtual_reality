#pragma once

#include "check.hpp"

#include <openxr/openxr.h>
#include <string>
#include <format>
#include <memory>
#include <utility>

namespace vr {

    struct Path {
        explicit Path(std::string value)
                : m_value{std::move(value)} {}

        virtual ~Path() = default;

        [[nodiscard]]
        virtual std::string value() const { return "/" + m_value; };

    private:
        std::string m_value;
    };


    struct CompositePath : public Path {
        CompositePath(std::string value, std::unique_ptr<Path> subPath)
                : Path{std::move(value)}, m_subPath{std::move(subPath)} {}

        [[nodiscard]]
        std::string value() const final {
            return Path::value() + m_subPath->value();
        }

        XrPath toXrPath(XrInstance instance) const {
            XrPath path;
            LOG_ERROR(instance, xrStringToPath(instance, value().c_str(), &path));
            return path;
        }

    private:
        std::unique_ptr<Path> m_subPath;
    };

    struct Input;


    struct Hand {
        std::string parent;
        std::string hand;

        [[nodiscard]]
        Hand left() {
            return {parent, "left"};
        }

        [[nodiscard]]
        Hand right() {
            return {parent, "right"};
        }

        Input input();
    };


    struct User {

        [[nodiscard]]
        static Hand hand() {
            return {"user"};
        }
    };

    struct Grip {
        Hand hand;

        [[nodiscard]]
        CompositePath pose() const {
            return CompositePath {
                hand.parent,
                std::make_unique<CompositePath>( hand.hand, std::make_unique<Path>("input/grip/pose"))
            };
        }
    };

    struct Select {
        Hand hand;

        [[nodiscard]]
        CompositePath click() const {
            return CompositePath {
                hand.parent,
                std::make_unique<CompositePath>(hand.hand, std::make_unique<Path>("input/select/click"))
            };
        }
    };

    struct Menu {
        Hand hand;

        [[nodiscard]]
        CompositePath click() const {
            return CompositePath {
                hand.parent,
                std::make_unique<CompositePath>(hand.hand, std::make_unique<Path>("input/menu/click"))
            };
        }
    };

    struct Aim {
        Hand hand;

        [[nodiscard]]
        CompositePath pose() const {
            return CompositePath {
                hand.parent,
                std::make_unique<CompositePath>( hand.hand, std::make_unique<Path>("input/aim/pose"))
            };
        }
    };

    struct Input {
        Hand hand;

        Grip grip() {
            return { hand };
        }

        Select select() {
            return { hand };
        }

        Menu menu() {
            return { hand };
        }
    };


    inline Input Hand::input() {
        return { *this };
    }
}