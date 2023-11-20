#pragma once

#include "vr/graphics/Renderer.hpp"
#include "util/ExponentialBackoff.hpp"
#include "SessionConfig.hpp"

#include <memory>
#include <functional>


namespace vr {

    using GraphicsFactory = std::function<std::shared_ptr<GraphicsService>(const Context&)>;

    class Application {
    public:
        Application(
                const ContextCreation& creation
                , const SessionConfig& sessionConfig
                , std::shared_ptr<Renderer> renderer
                , GraphicsFactory&& graphicsFactory);

        void run();

        void shutdown();

        static bool pollEvents(const Context& context, XrEventDataBuffer& event);

    private:
        std::shared_ptr<Renderer> m_renderer;
        GraphicsFactory createGraphicsService;
        const ContextCreation& m_contextCreator;
        const SessionConfig& m_sessionConfig;
        util::Backoff backoff;

    };
}