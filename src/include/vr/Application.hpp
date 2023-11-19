#pragma once

#include "vr/graphics/Renderer.hpp"
#include "util/ExponentialBackoff.hpp"

#include <memory>
#include <functional>


namespace vr {

    using GraphicsFactory = std::function<std::shared_ptr<GraphicsService>(const Context&)>;

    class Application {
    public:
        Application(
                std::shared_ptr<Renderer> renderer
                , GraphicsFactory&& graphicsFactory
                , const ContextCreation& creation);

        void run();

        void shutdown();

        static bool pollEvents(const Context& context, XrEventDataBuffer& event);

    private:
        std::shared_ptr<Renderer> m_renderer;
        GraphicsFactory createGraphicsService;
        const ContextCreation& m_contextCreator;
        util::Backoff backoff;

    };
}