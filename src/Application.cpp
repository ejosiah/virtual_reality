#include "vr/Application.hpp"
#include "vr/SessionService.hpp"
#include "xr_struct_mapping.hpp"
#include "vr/ToString.hpp"

#include <spdlog/spdlog.h>

#include <cmath>
#include <thread>
#include <chrono>

namespace vr {

    Application::Application(
            const ContextCreation &creation,
            const SessionConfig& sessionConfig,
            std::shared_ptr<Renderer> renderer,
            GraphicsFactory &&graphicsFactory

    )
    : m_contextCreator(creation)
    , m_sessionConfig(sessionConfig)
    , m_renderer(std::move(renderer))
    , createGraphicsService(std::move(graphicsFactory))
    , backoff("failed to create XR instance")
    {}

    void Application::run() {
        do {
            auto context = m_contextCreator.create();

            if(!context){
                backoff();
                continue;
            }else {
                backoff.reset();
            }
            auto graphicsService = createGraphicsService(*context);
            graphicsService->init();
            m_renderer->set(*context, graphicsService);

            SessionService session{ *context, m_sessionConfig, graphicsService, m_renderer };
            session.create();

            auto event = makeStruct<XrEventDataBuffer>();
            while(session.isRunning()) {

                if(pollEvents(*context, event)) {
                    switch (event.type) {
                        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED :
                            session.handle(event);
                            break;
                        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING :
                            session.stop();
                            break;
                        default:
                            THROW("Event type not yet implemented!");
                    }
                }

                session.processFrame();
            }

            graphicsService->shutdown();
            context->destroy();

            if(session.shouldExit()) {
                break;
            }
        } while (true);

        spdlog::info("XR instance terminated, exiting application");
    }

    bool Application::pollEvents(const Context& context, XrEventDataBuffer& event) {
        event.type = XR_TYPE_EVENT_DATA_BUFFER;
        auto result = xrPollEvent(context.instance, &event);
        return result != XR_EVENT_UNAVAILABLE;
    }

    void Application::shutdown() {
    }

}
