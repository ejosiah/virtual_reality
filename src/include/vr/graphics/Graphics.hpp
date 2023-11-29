#pragma once

#include "vr/Context.hpp"
#include "vr/Models.hpp"
#include "vr/WindowingSystem.hpp"

#include <openxr/openxr.h>
#include <glm/glm.hpp>

#include <utility>
#include <vector>
#include <memory>

namespace vr {
    struct GraphicsService {
        explicit GraphicsService(const Context& context)
        : m_context(context)
        {}

        virtual ~GraphicsService() = default;

        virtual void init() = 0;

        [[nodiscard]]
        virtual const XrBaseInStructure& graphicsBinding() const = 0;

        [[nodiscard]] const Context& context() const {
            return m_context;
        }

        virtual void shutdown() {}

        virtual void setSwapChains(std::vector<SwapChain> swapchains) = 0;

        virtual glm::mat4 projection(const XrFovf &fov, float zNear, float zFar) {
            return glm::mat4{1};
        }

#ifdef USE_MIRROR_WINDOW
        virtual void initMirrorWindow() {};

        virtual void mirror(const ImageId& imageId) {}

        virtual void shutdownMirrorWindow() {};

        virtual Window& window() = 0;
#endif

    protected:
        const Context& m_context;
    };
}