#pragma once

#include <vr/Context.hpp>
#include <openxr/openxr.h>

#include <utility>
#include <vector>
#include <memory>

namespace vr {
    struct GraphicsService {
        GraphicsService(const Context& context)
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

        virtual void setSwapChainImages(XrSwapchain swapchain) = 0;

        [[nodiscard]]
        virtual int64_t swapChainFormat() const  = 0;

    protected:
        const Context& m_context;


    };
}