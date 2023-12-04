#pragma once

#include "vr/Models.hpp"
#include <openxr/openxr.h>

#include <format>
#include <set>

namespace vr {

    struct ActionSpecification {
        std::string name{};
        std::string description;
        XrActionType type{};
        std::string path{};

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

        ActionSetSpecification& addAction(std::string_view name, XrActionType actionType, std::string_view path, std::string_view description = "") {
            description = description.empty() ? name : description;
            _actions.insert( ActionSpecification{name.data(), description.data(), actionType, path.data()});
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
                if(!static_cast<int>(action.type)) {
                    THROW(std::format("Action type is required for action: {}.{}",_name, action.name));
                }
                if(action.path.empty()){
                    THROW(std::format("path is required for action: {}.{}",_name, action.name));
                }
            }
        }

    };

}