#pragma once

#include "check.hpp"
#include "forwards.hpp"
#include "xr_struct_mapping.hpp"
#include "Creation.hpp"
#include "Enumerators.hpp"

#include <openxr_ext_loader.h>
#include <vulkan/vulkan.h>
#include <openxr/openxr.h>
#include <spdlog/spdlog.h>

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <optional>

namespace vr {

    struct Context {
        XrInstance instance{XR_NULL_HANDLE};
        XrSystemId systemId{XR_NULL_SYSTEM_ID};
        std::shared_ptr<GraphicsContext> graphicsContext;
#ifndef NDEBUG
#ifdef XR_DEBUG
      XrDebugUtilsMessengerEXT debugMessenger{XR_NULL_HANDLE};
#endif
#endif

      [[nodiscard]]
      bool isSupported(XrViewConfigurationType type) const;

      [[nodiscard]]
      std::vector<XrViewConfigurationView> views(XrViewConfigurationType viewType) const;

      void destroy();
    };

    std::optional<Context> createContext(const ContextCreation& creation);
}