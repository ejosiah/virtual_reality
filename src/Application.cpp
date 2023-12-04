#include "vr/Application.hpp"
#include "vr/SessionService.hpp"
#include "xr_struct_mapping.hpp"
#include "vr/ToString.hpp"

#ifdef USE_MIRROR_WINDOW
#include "vr/WindowingSystem.hpp"
#endif

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
#ifdef USE_MIRROR_WINDOW
        WindowingSystem::init();
#endif
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
            session.init();

            auto event = makeStruct<XrEventDataBuffer>();
            while(session.isRunning()) {

#ifdef USE_MIRROR_WINDOW
                WindowingSystem::pollEvents();
#endif

                if(pollEvents(*context, event)) {
                    switch (event.type) {
                        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED :
                            session.handle(event);
                            break;
                        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING :
                            session.stop();
                            break;
                        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
                            break;
                        default: {
                            char buffer[XR_MAX_STRUCTURE_NAME_SIZE];
                            xrStructureTypeToString(context->instance, event.type, buffer);
                            THROW(std::format("Event[{}] type not yet implemented!", buffer));
                        }
                    }
                }

                session.processFrame();
            }

#ifdef USE_MIRROR_WINDOW
            graphicsService->shutdownMirrorWindow();
#endif

            graphicsService->shutdown();
            context->destroy();

            if(session.shouldExit()) {
                break;
            }
        } while (true);

#ifdef USE_MIRROR_WINDOW
        WindowingSystem::shutdown();
#endif

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
