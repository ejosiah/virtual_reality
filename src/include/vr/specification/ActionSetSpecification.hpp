#pragma once

#include "vr/Models.hpp"
#include "vr/Input.hpp"

#include <openxr/openxr.h>

#include <format>
#include <set>

namespace vr {

    struct ActionSpecification {
        std::string name{};
        std::string description;
        vr::input::Input input;

        auto operator<=>(const ActionSpecification& other) const {
            return name<=>other.name;
        }
    };

    struct ActionSetSpecification {
        std::string _name;
        std::string _description;
        std::set<ActionSpecification> _actions;

        ActionSetSpecification& name(std::string_view aName) {
            _name = aName;
            return *this;
        }

        ActionSetSpecification& description(std::string_view desc) {
            _description = desc;
            return *this;
        }

        ActionSetSpecification& addAction(std::string_view name
                                          , vr::input::Source source
                                          , vr::input::Identifier identifier
                                          , vr::input::Component component
                                          , std::string_view description = "") {

            description = description.empty() ? name : description;
            _actions.insert( ActionSpecification{name.data(), description.data(), { source, identifier, component}} );
            return *this;
        }

        void validate() const {
            if(_name.empty()) {
                THROW("ActionSet name is required");
            }
            for(const auto& action : _actions) {
                if(action.name.empty()) {
                    THROW("Action name is required");
                }
            }
        }

    };

}