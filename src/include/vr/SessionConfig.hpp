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
        // TODO action spaces
        // TODO actions

        SessionConfig& addSwapChain(const SwapchainSpecification& spec) {
            _swapchains.push_back(spec);
            return *this;
        }

        SessionConfig& addSpace(const ReferenceSpaceSpecification& spec) {
            _spaces.push_back(spec);
            return *this;
        }

        void validate(const vr::Context& context) const {
            if(_swapchains.empty()) {
                THROW("at least one swapchain should be provided")
            }
        }
    };
}