#pragma once

#include "Types.hpp"
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

        virtual void set(const std::vector<SpaceLocation>& spaceLocations) {

        }

        virtual void init() {}

        virtual cstring name() { return ""; }

        virtual FrameEnd paused(const FrameInfo &frameInfo) = 0;

        virtual FrameEnd render(const FrameInfo &frameInfo) = 0;

    protected:
        std::shared_ptr<GraphicsService> m_graphics;
        Context m_context;
    };

    class VoidRenderer : public Renderer {
    public:
        ~VoidRenderer() override = default;

        FrameEnd paused(const FrameInfo &frameInfo) override {
            return {};
        }

        FrameEnd render(const FrameInfo &frameInfo) override {
            return {};
        }

        static std::shared_ptr<Renderer> shared() {
            return std::make_shared<VoidRenderer>();
        }
    };
}
