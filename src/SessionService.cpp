#include "check.hpp"
#include "vr/SessionService.hpp"
#include "xr_struct_mapping.hpp"
#include "vr/Enumerators.hpp"
#include "vr/ToString.hpp"

#include <stdexcept>
#include <thread>
#include <chrono>

namespace vr {

    SessionService::SessionService(
            const Context &context
            , const SessionConfig& config
            , std::shared_ptr<GraphicsService> graphics
            , std::shared_ptr<Renderer> renderer)
    : m_ctx(context)
    , m_config(config)
    , m_graphics(std::move(graphics))
    , m_renderer(std::move(renderer))
    {
        m_states[XR_SESSION_STATE_UNKNOWN] = std::make_unique<SessionState>(*this);
        m_states[XR_SESSION_STATE_IDLE] = std::make_unique<SessionIdle>(*this);
        m_states[XR_SESSION_STATE_READY] = std::make_unique<SessionReady>(*this);
        m_states[XR_SESSION_STATE_SYNCHRONIZED] = std::make_unique<SessionSynchronized>(*this);
        m_states[XR_SESSION_STATE_VISIBLE] = std::make_unique<SessionVisible>(*this);
        m_states[XR_SESSION_STATE_FOCUSED] = std::make_unique<SessionFocused>(*this);
        m_states[XR_SESSION_STATE_STOPPING] = std::make_unique<SessionStopping>(*this);
        m_states[XR_SESSION_STATE_LOSS_PENDING] = std::make_unique<SessionLossPending>(*this);
        m_states[XR_SESSION_STATE_EXITING] = std::make_unique<SessionExiting>(*this);
    }

    void SessionService::create() {
        m_config.validate(m_ctx);
        auto createInfo = makeStruct<XrSessionCreateInfo>();
        const auto& graphicsBinding = m_graphics->graphicsBinding();
        createInfo.systemId = m_ctx.systemId;
        createInfo.next = &graphicsBinding;
        LOG_ERROR(m_ctx.instance, xrCreateSession(m_ctx.instance, &createInfo, &m_session))
        createSwapChain();
        createMainViewSpace();
        m_renderer->init();
        m_currentState = XR_SESSION_STATE_UNKNOWN;
    }
    
    void SessionService::createMainViewSpace() {
        auto createInfo = makeStruct<XrReferenceSpaceCreateInfo>();
        createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
        createInfo.poseInReferenceSpace.position = {0, 0, 0};
        createInfo.poseInReferenceSpace.orientation = {0, 0, 0, 1};
        LOG_ERROR(m_ctx.instance, xrCreateReferenceSpace(m_session, &createInfo, &m_mainViewSpace))
    }

    void SessionService::handle(const XrEventDataBuffer &event) {
        if(event.type != XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
            throw std::runtime_error{"Event is not a session state changed event"};
        }
        m_states[m_currentState]->handle(reinterpret_cast<const XrEventDataSessionStateChanged&>(event));
    }
    
    void SessionService::processFrame() {
        m_states[m_currentState]->processFrame();
    }

    void SessionService::createSwapChain() {

        int64_t format = m_graphics->swapChainFormat();

        if(!imageFormatIsSupported(format)){
            throw std::runtime_error{"chosen swapchain image format unsupported"};
        }

        for(const auto& spec : m_config._swapchains) {
            XrSwapchainCreateInfo createInfo = makeStruct<XrSwapchainCreateInfo>();
            createInfo.usageFlags = spec._usageFlags;
            createInfo.format = spec._format;
            createInfo.sampleCount = spec._sampleCount;
            createInfo.width = spec._width;
            createInfo.height = spec._height;
            createInfo.faceCount = spec._faceCount;
            createInfo.arraySize = spec._arraySize;
            createInfo.mipCount = spec._mipCount;

            XrSwapchain swapchain;
            LOG_ERROR(m_ctx.instance, xrCreateSwapchain(m_session, &createInfo, &swapchain))

            m_swapchains.push_back({ spec, swapchain});
        }
        m_graphics->setSwapChains(m_swapchains);
    }
    
    void SessionService::stop() {
        transitionTo(XR_SESSION_STATE_LOSS_PENDING);
        xrDestroySession(m_session);
    }

    void SessionService::transitionTo(XrSessionState state) {
        if(state == XR_SESSION_STATE_UNKNOWN) throw std::runtime_error{"Invalid state"};
        spdlog::debug("transitioning session state from {} to {}", toString(m_currentState), toString(state));
        m_currentState = state;
    }

    bool SessionService::imageFormatIsSupported(uint64_t format) {
        auto [result, formats] = enumerate<int64_t>(m_session, xrEnumerateSwapchainFormats);
        CHECK_XR(result);
        return std::any_of(formats.begin(), formats.end(), [format](auto supportedFormat){ return supportedFormat == format; });
    }

    void SessionStateRunning::processFrame() {
        if(state() == XR_SESSION_STATE_UNKNOWN) {
            throw std::runtime_error{"Invalid state"};
        }

        static FrameEnd frame{};
        frame.layers.clear();

        const auto session = m_sessionService.m_session;
        const auto swapchain = m_sessionService.m_swapchains.front();

        static auto frameState = makeStruct<XrFrameState>();
        xrWaitFrame(session, XR_NULL_HANDLE, &frameState);


        if(XR_SUCCEEDED(xrBeginFrame(session, nullptr))) {
            if(frameState.shouldRender){
                uint32_t imageIndex;
                xrAcquireSwapchainImage(swapchain.handle, nullptr, &imageIndex);

                auto waitInfo = makeStruct<XrSwapchainImageWaitInfo>();
                waitInfo.timeout = XR_INFINITE_DURATION;

                if (XR_UNQUALIFIED_SUCCESS(xrWaitSwapchainImage(swapchain.handle, &waitInfo))) {

                    auto viewState = makeStruct<XrViewState>();
                    auto viewLocateInfo = makeStruct<XrViewLocateInfo>();
                    viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                    viewLocateInfo.displayTime = frameState.predictedDisplayTime;
                    viewLocateInfo.space = m_sessionService.m_mainViewSpace;
                    uint32_t numViews;
                    xrLocateViews(session, &viewLocateInfo, &viewState, 0, &numViews, XR_NULL_HANDLE);
                    std::vector<XrView> views(numViews, { XR_TYPE_VIEW});
                    LOG_ERROR(m_sessionService.m_ctx.instance, xrLocateViews(session, &viewLocateInfo, &viewState, numViews, &numViews, views.data())) ;
                    ViewInfo viewInfo{viewState, views};

                    frame = frameLoop({{swapchain.spec._name, imageIndex}, {viewState, std::move(views)},
                                       m_sessionService.m_mainViewSpace});

                }
                xrReleaseSwapchainImage(swapchain.handle, nullptr);
            }

            XrFrameEndInfo endFrameInfo = makeStruct<XrFrameEndInfo>();
            endFrameInfo.environmentBlendMode = frame.blendMode;
            endFrameInfo.layerCount = frame.layers.size();
            endFrameInfo.layers = frame.layers.data();
            endFrameInfo.displayTime = frameState.predictedDisplayTime;
            xrEndFrame(session, &endFrameInfo);
        }

    }

    void SessionStateRunning::handle(const XrEventDataSessionStateChanged &event) {
        if(event.state == XR_SESSION_STATE_STOPPING) {
            m_sessionService.transitionTo(XR_SESSION_STATE_STOPPING);
            xrEndSession(m_sessionService.m_session);
        }else {
            SessionState::handle(event);
        }
    }

    void SessionIdle::handle(const XrEventDataSessionStateChanged &event) {
        const auto state = event.state;
        if(state == XR_SESSION_STATE_READY) {
            m_sessionService.transitionTo(state);
            if (!m_sessionService.m_ctx.isSupported(XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO)) {
                THROW("Stereo configuration required")
            }

            auto beginInfo = makeStruct<XrSessionBeginInfo>();
            beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
            auto result =  xrBeginSession(m_sessionService.m_session, &beginInfo);
            LOG_ERROR(m_sessionService.m_ctx.instance, result)
            spdlog::debug("XR session begun");
        }else if(state == XR_SESSION_STATE_EXITING || state == XR_SESSION_STATE_LOSS_PENDING) {
            m_sessionService.transitionTo(state);
            CHECK_XR(xrDestroySession(m_sessionService.m_session));
        }else {
            SessionState::handle(event);
        }
    }


    void SessionIdle::processFrame() {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
    }


    FrameEnd SessionVisible::frameLoop(const FrameInfo &frameInfo) {
        return m_sessionService.m_renderer->paused(frameInfo);
    }

    FrameEnd SessionFocused::frameLoop(const FrameInfo &frameInfo) {
        return m_sessionService.m_renderer->render(frameInfo);
    }

}
