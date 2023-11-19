#pragma once

#include "vr/Models.hpp"
#include "Graphics.hpp"

#include <utility>
#include <memory>


namespace vr {

    class Renderer {
    public:
        virtual ~Renderer() = default;

        void set(Context context, std::shared_ptr<GraphicsService> graphics) {
            m_context = std::move(context);
            m_graphics = std::move(graphics);
        }

        virtual FrameEnd paused(uint32_t imageIndex, const ViewInfo& viewInfo) = 0;

        virtual FrameEnd render(uint32_t imageIndex, const ViewInfo& viewInfo) = 0;

    protected:
        std::shared_ptr<GraphicsService> m_graphics;
        Context m_context;
    };

    class VoidRenderer : public Renderer {
    public:
        ~VoidRenderer() override = default;

        FrameEnd paused(uint32_t imageIndex, const ViewInfo &viewInfo) override {
            return {};
        }

        FrameEnd render(uint32_t imageIndex, const ViewInfo &viewInfo) override {
            return {};
        }

        static std::shared_ptr<Renderer> shared() {
            return std::make_shared<VoidRenderer>();
        }
    };
}
