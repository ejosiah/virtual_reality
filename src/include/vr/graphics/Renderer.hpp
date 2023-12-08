#pragma once

#include "Types.hpp"
#include "vr/Models.hpp"
#include "Graphics.hpp"
#include "vr/Action.hpp"

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

        virtual void beginFrame() {}

        virtual void endFrame() {}

        virtual void set(const std::vector<SpaceLocation>& spaceLocations) {}

        virtual std::vector<Vibrate> set(const ActionSet& actionSet) {
            return {};
        }

        virtual void init() {}

        virtual void cleanup() {}

        virtual cstring name() { return ""; }

        virtual void paused(const FrameInfo &frameInfo, Layers& layers) = 0;

        virtual void render(const FrameInfo &frameInfo, Layers& layers) = 0;

        virtual std::string activeActionSets() { return "*"; };

    protected:
        std::shared_ptr<GraphicsService> m_graphics;
        Context m_context;
    };

    class VoidRenderer : public Renderer {
    public:
        ~VoidRenderer() override = default;

        void paused(const FrameInfo &frameInfo, Layers& layers) override {}

        void render(const FrameInfo &frameInfo, Layers& layers) override {}

        static std::shared_ptr<Renderer> shared() {
            return std::make_shared<VoidRenderer>();
        }
    };
}
