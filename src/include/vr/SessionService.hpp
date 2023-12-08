#pragma once

#include "Context.hpp"
#include "Models.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/Renderer.hpp"
#include "SessionConfig.hpp"
#include "Input.hpp"

#include <openxr/openxr.h>

#include <unordered_map>
#include <memory>
#include <span>

namespace vr {

    struct ActionData {
        vr::Input input;
        XrAction _;
    };

    struct ActionSetBinding {
        XrActionSet xrActionSet{};
        ActionSet& actionSet;
        std::map<std::string, ActionData> actions;
    };

    class SessionService {
    public:
        friend class SessionState;
        friend class SessionStateRunning;
        friend class SessionIdle;
        friend class SessionReady;
        friend class SessionSynchronized;
        friend class SessionVisible;
        friend class SessionFocused;
        friend class SessionStopping;
        friend class SessionLossPending;

        SessionService(
                const Context &context
                , const SessionConfig& config
                , std::shared_ptr<GraphicsService> graphics
                , std::shared_ptr<Renderer> renderer);

        void init();

        void handle(const XrEventDataBuffer &event);
        
        void processFrame();

        void stop();

        void terminate();

        void handleHostKeyPress(int key, int scancode, int action, int mods);

        bool imageFormatIsSupported(uint64_t format);

        [[nodiscard]] bool isRunning() const {
            return   m_currentState != XR_SESSION_STATE_EXITING && m_currentState != XR_SESSION_STATE_LOSS_PENDING;
        }

        [[nodiscard]] bool shouldExit() const {
            return m_currentState == XR_SESSION_STATE_EXITING;
        }

        [[nodiscard]] const XrSession get() const {
            return m_session;
        }

        
    private:
        void createSession();

        void createReferenceSpaces();

        void setupActions();

        void bindActions(const InteractionProfile& profile);

        void createSwapChain();

        void createMainViewSpace();

        void initRenderer();

        void transitionTo(XrSessionState state);

    private:
        const Context& m_ctx;
        const SessionConfig& m_config;
        std::shared_ptr<GraphicsService> m_graphics;
        std::shared_ptr<Renderer> m_renderer;
        XrSession m_session{};
        std::vector<SwapChain> m_swapchains;
        XrSessionState m_currentState{XR_SESSION_STATE_UNKNOWN};
        std::unordered_map<XrSessionState, std::unique_ptr<SessionState>> m_states;
        std::vector<XrView> m_views;
        XrSpace m_baseSpace{};
        std::map<std::string, XrSpace> m_spaces;
        bool m_terminationRequested{};
        std::vector<ActionSetBinding> m_actionSetBindings;
        std::vector<ActionSet> m_actionSets;
        std::vector<XrActiveActionSet> m_activeActionSets;
        std::map<std::string, XrSpace> m_actionSpaces;
        std::vector<ImageId> m_swapChainImages;
        XrEnvironmentBlendMode m_blendMode{XR_ENVIRONMENT_BLEND_MODE_OPAQUE};
    };

    class SessionState {
    public:
        explicit SessionState(SessionService& sessionService)
        : m_sessionService(sessionService) {}

        virtual ~SessionState() = default;

        virtual void handle(const XrEventDataSessionStateChanged& event) {
            if(event.state == state()) return;
            m_sessionService.transitionTo(event.state);
        };

        virtual void processFrame() {}

        virtual void frameLoop(const FrameInfo &frameInfo, Layers& layers) {
        }

        [[nodiscard]]
        virtual XrSessionState state() const { return XR_SESSION_STATE_UNKNOWN; }

    protected:
        SessionService& m_sessionService;
    };

    class SessionStateRunning : public SessionState {
    public:
        explicit SessionStateRunning(SessionService& sessionService)
        : SessionState(sessionService) {};

        virtual void beginFrame();

        void processFrame() override;

        virtual void endFrame();

        void handle(const XrEventDataSessionStateChanged &event) final;

    protected:
        XrFrameState m_frameState{ XR_TYPE_FRAME_STATE };

    };

    class SessionIdle : public SessionState {
    public:
        explicit SessionIdle(SessionService& sessionService)
        : SessionState(sessionService) {};

        void handle(const XrEventDataSessionStateChanged &event) final;

        void processFrame() override;

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_IDLE; }
    };

    class SessionReady : public SessionStateRunning {
    public:
        explicit SessionReady(SessionService& sessionService)
        :SessionStateRunning(sessionService){};

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_READY; }

    };

    class SessionSynchronized : public SessionStateRunning {
    public:
        explicit SessionSynchronized(SessionService& sessionService)
        : SessionStateRunning(sessionService){};

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_SYNCHRONIZED; }

    };

    class SessionVisible : public SessionStateRunning {
    public:
        explicit SessionVisible(SessionService& sessionService)
        : SessionStateRunning(sessionService){};

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_VISIBLE; }

        void frameLoop(const FrameInfo &frameInfo, Layers& layers) final;
    };

    class SessionFocused : public SessionStateRunning {
    public:
        explicit SessionFocused(SessionService& sessionService)
        : SessionStateRunning(sessionService){};

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_FOCUSED; }

        void beginFrame() final {};

        void processFrame() final;

        void frameLoop(const FrameInfo &frameInfo, Layers& layers) final;
    };

    class SessionStopping : public SessionState {
    public:
        explicit SessionStopping(SessionService& sessionService)
        : SessionState(sessionService){};

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_STOPPING; }

    };

    class SessionLossPending : public SessionState {
    public:
        explicit SessionLossPending(SessionService& sessionService)
        : SessionState(sessionService){};

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_LOSS_PENDING; }

    };

    class SessionExiting : public SessionState {
    public:
        explicit SessionExiting(SessionService& sessionService)
        : SessionState(sessionService){};

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_EXITING; }

    };


}
