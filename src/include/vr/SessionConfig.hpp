#pragma once

#include "check.hpp"
#include "specification/Specifications.hpp"

#include <openxr/openxr.h>

#include <vector>
#include <string>

namespace vr {

    struct SessionConfig {
        std::vector<SwapchainSpecification> _swapchains;
        std::vector<ReferenceSpaceSpecification> _spaces;
        std::vector<ActionSetSpecification> _actionSets;

        SessionConfig& addSwapChain(const SwapchainSpecification& spec) {
            _swapchains.push_back(spec);
            return *this;
        }

        SessionConfig& addSpace(const ReferenceSpaceSpecification& spec) {
            _spaces.push_back(spec);
            return *this;
        }

        SessionConfig& addActionSet(const ActionSetSpecification& spec) {
            _actionSets.push_back(spec);
            return *this;
        }

        void validate(const vr::Context& context) const {
            if(_swapchains.empty()) {
                THROW("at least one swapchain should be provided")
            }
            for(const auto& actionSet : _actionSets) {
                actionSet.validate();
            }
        }
    };
}