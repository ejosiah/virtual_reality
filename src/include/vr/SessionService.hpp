#pragma once

#include "Context.hpp"
#include "Models.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/Renderer.hpp"
#include "SessionConfig.hpp"

#include <openxr/openxr.h>

#include <unordered_map>
#include <memory>
#include <span>

namespace vr {

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

        void create();

        void createSwapChain();

        void createMainViewSpace();

        void handle(const XrEventDataBuffer &event);
        
        void processFrame();

        void stop();

        bool imageFormatIsSupported(uint64_t format);

        [[nodiscard]] bool isRunning() const {
            return   m_currentState != XR_SESSION_STATE_EXITING && m_currentState != XR_SESSION_STATE_LOSS_PENDING;
        }

        [[nodiscard]] bool shouldExit() const {
            return m_currentState == XR_SESSION_STATE_EXITING;
        }

        
    private:
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
        std::vector<XrView> views;
        XrSpace m_mainViewSpace{};
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

        virtual FrameEnd frameLoop(const FrameInfo &frameInfo) {
            return {};
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

        void processFrame() final;

        void handle(const XrEventDataSessionStateChanged &event) final;

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

        FrameEnd frameLoop(const FrameInfo &frameInfo) final;
    };

    class SessionFocused : public SessionStateRunning {
    public:
        explicit SessionFocused(SessionService& sessionService)
        : SessionStateRunning(sessionService){};

        [[nodiscard]]
        XrSessionState state() const final { return XR_SESSION_STATE_FOCUSED; }

        FrameEnd frameLoop(const FrameInfo &frameInfo) final;
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
