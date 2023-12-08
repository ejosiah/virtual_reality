#include "check.hpp"
#include "vr/SessionService.hpp"
#include "xr_struct_mapping.hpp"
#include "vr/Enumerators.hpp"
#include "vr/ToString.hpp"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <thread>
#include <chrono>

namespace vr {

    void OnHostKeyPress(GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto session = reinterpret_cast<SessionService*>(glfwGetWindowUserPointer(window));
        session->handleHostKeyPress(key, scancode, action, mods);
    }

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

    void SessionService::init() {
        m_config.validate(m_ctx);
        createSession();
        createReferenceSpaces();
        createSwapChain();
        createMainViewSpace();
        setupActions();
        initRenderer();
#ifdef USE_MIRROR_WINDOW
        m_graphics->initMirrorWindow();
        auto window = m_graphics->window();
        glfwSetWindowUserPointer(window._, this);
        glfwSetKeyCallback(window._, OnHostKeyPress);
#endif
    }

    void SessionService::createSession() {
        auto createInfo = makeStruct<XrSessionCreateInfo>();
        const auto& graphicsBinding = m_graphics->graphicsBinding();
        createInfo.systemId = m_ctx.systemId;
        createInfo.next = &graphicsBinding;
        LOG_ERROR(m_ctx.instance, xrCreateSession(m_ctx.instance, &createInfo, &m_session))
    }

    void SessionService::createReferenceSpaces() {
        for(const auto& spec : m_config._spaces) {
            auto createInfo = makeStruct<XrReferenceSpaceCreateInfo>();
            createInfo.referenceSpaceType = spec._referenceSpaceType;
            createInfo.poseInReferenceSpace = convert(spec._poseInReferenceSpace);

            XrSpace space;
            CHECK_XR(xrCreateReferenceSpace(m_session, &createInfo, &space));

            m_spaces.insert(std::make_pair(spec._name, space));
        }
    }
    
    void SessionService::createMainViewSpace() {
        auto createInfo = makeStruct<XrReferenceSpaceCreateInfo>();
        createInfo.referenceSpaceType = m_config._baseSpaceType;
        createInfo.poseInReferenceSpace.position = {0, 0, 0};
        createInfo.poseInReferenceSpace.orientation = {0, 0, 0, 1};
        LOG_ERROR(m_ctx.instance, xrCreateReferenceSpace(m_session, &createInfo, &m_baseSpace))
    }

    void SessionService::initRenderer() {
        m_renderer->init();
    }

    void SessionService::handle(const XrEventDataBuffer &event) {
        if(event.type != XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
            throw std::runtime_error{"Event is not a session state changed event"};
        }
        m_states[m_currentState]->handle(reinterpret_cast<const XrEventDataSessionStateChanged&>(event));
    }
    
    void SessionService::processFrame() {
        m_states[m_currentState]->processFrame();
        if(m_terminationRequested) {
            stop();
            m_currentState = XR_SESSION_STATE_EXITING;
        }
    }

    void SessionService::createSwapChain() {

        for(const auto& spec : m_config._swapchains) {
            if(!imageFormatIsSupported(spec._format)){
                throw std::runtime_error{"chosen swapchain image format unsupported"};
            }
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
        m_swapChainImages.resize(m_swapchains.size());
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

    void SessionService::handleHostKeyPress(int key, int scancode, int action, int mods) {
        if(action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
            terminate();
        }
    }

    void SessionService::terminate() {
        m_terminationRequested = true;
    }

    void SessionService::setupActions() {
        if(m_config._actionSets.empty()){
            spdlog::info("No action sets to configure");
            return;
        }
        std::vector<XrActionSuggestedBinding> actionSuggestedBinding;
        std::vector<XrActionSet> actionSets;

        for(const auto& spec : m_config._actionSets) {
            auto createInfo = makeStruct<XrActionSetCreateInfo>();
            strcpy_s(createInfo.actionSetName, spec._name.c_str());

            auto desc = spec._description.empty() ? spec._name : spec._description;
            strcpy_s(createInfo.localizedActionSetName, desc.c_str());

            XrActionSet actionSet;
            CHECK_XR(xrCreateActionSet(m_ctx.instance, &createInfo, &actionSet));
            actionSets.push_back(actionSet);

            m_actionSets.emplace_back();
            m_actionSetBindings.push_back({actionSet, m_actionSets.back()});
            ActionSetBinding& binding = m_actionSetBindings.back();
            
            for(const auto& actionSpec : spec._actions) {
                spdlog::debug("configuration action {} in action set {}", actionSpec.name, spec._name);
                auto actionInfo = makeStruct<XrActionCreateInfo>();
                strcpy_s(actionInfo.actionName, actionSpec.name.c_str());
                strcpy_s(actionInfo.localizedActionName, actionSpec.description.c_str());
                actionInfo.actionType = actionSpec.type;

                XrAction action;
                CHECK_XR(xrCreateAction(actionSet, &actionInfo, &action));
                
                if(actionSpec.type == XR_ACTION_TYPE_POSE_INPUT) {
                    auto spaceInfo = makeStruct<XrActionSpaceCreateInfo>();
                    spaceInfo.action = action;
                    spaceInfo.poseInActionSpace.position = {0, 0, 0};
                    spaceInfo.poseInActionSpace.orientation = {0, 0, 0, 1};
                    XrSpace space;
                    xrCreateActionSpace(m_session, &spaceInfo, &space);
                    m_actionSpaces[actionSpec.name] = space;
                }

                XrPath path;
                xrStringToPath(m_ctx.instance, actionSpec.path.c_str(), &path);
                binding.actions[actionSpec.name] =  { path, actionSpec.type, action };
                binding.actionSet.actions[actionSpec.name] = {actionSpec.name, false};

                actionSuggestedBinding.push_back({action, path});
            }
        }

        auto suggestedBindings = makeStruct<XrInteractionProfileSuggestedBinding>();
        XrPath profile;
//        xrStringToPath(m_ctx.instance, "/interaction_profiles/khr/simple_controller", &profile);
        xrStringToPath(m_ctx.instance, "/interaction_profiles/oculus/touch_controller", &profile);
        suggestedBindings.interactionProfile = profile;
        suggestedBindings.countSuggestedBindings = actionSuggestedBinding.size();
        suggestedBindings.suggestedBindings = actionSuggestedBinding.data();
        LOG_ERROR(m_ctx.instance, xrSuggestInteractionProfileBindings(m_ctx.instance, &suggestedBindings));

        auto attachInfo = makeStruct<XrSessionActionSetsAttachInfo>();
        attachInfo.countActionSets = actionSets.size();
        attachInfo.actionSets = actionSets.data();
        LOG_ERROR(m_ctx.instance, xrAttachSessionActionSets(m_session, &attachInfo));

        const auto activeSets = m_renderer->activeActionSets();
        if(activeSets == "*"){
            for(auto actionSet : actionSets){
                m_activeActionSets.push_back({ actionSet, XR_NULL_PATH});
            }
        }else {
            // TODO filter setA/SetB/SetC
        }
    }

    void SessionStateRunning::beginFrame() {
        m_sessionService.m_renderer->beginFrame();
    }

    void SessionStateRunning::processFrame() {
        if(state() == XR_SESSION_STATE_UNKNOWN) {
            throw std::runtime_error{"Invalid state"};
        }

        static FrameEnd frame{};
        frame.layers.clear();

        const auto session = m_sessionService.m_session;
        const auto swapchain = m_sessionService.m_swapchains.front();


        xrWaitFrame(session, XR_NULL_HANDLE, &m_frameState);


        if(XR_SUCCEEDED(xrBeginFrame(session, nullptr))) {
            if(m_frameState.shouldRender){
                beginFrame();
                uint32_t imageIndex;
                xrAcquireSwapchainImage(swapchain.handle, nullptr, &imageIndex);
                ImageId imageId{swapchain.spec._name, imageIndex};
                auto waitInfo = makeStruct<XrSwapchainImageWaitInfo>();
                waitInfo.timeout = XR_INFINITE_DURATION;

                if (XR_UNQUALIFIED_SUCCESS(xrWaitSwapchainImage(swapchain.handle, &waitInfo))) {

                    // Get view info
                    auto viewState = makeStruct<XrViewState>();
                    auto viewLocateInfo = makeStruct<XrViewLocateInfo>();
                    viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                    viewLocateInfo.displayTime = m_frameState.predictedDisplayTime;
                    viewLocateInfo.space = m_sessionService.m_baseSpace;
                    uint32_t numViews;
                    xrLocateViews(session, &viewLocateInfo, &viewState, 0, &numViews, XR_NULL_HANDLE);
                    std::vector<XrView> views(numViews, { XR_TYPE_VIEW});
                    LOG_ERROR(m_sessionService.m_ctx.instance, xrLocateViews(session, &viewLocateInfo, &viewState, numViews, &numViews, views.data())) ;
                    ViewInfo viewInfo{viewState, views};

                    // get space locations
                    std::vector<SpaceLocation> spaceLocations;
                    spaceLocations.reserve(m_sessionService.m_spaces.size());
                    for(const auto& [name, space] : m_sessionService.m_spaces) {
                        auto location = makeStruct<XrSpaceLocation>();
                        xrLocateSpace(space, m_sessionService.m_baseSpace, m_frameState.predictedDisplayTime, &location);
                        spaceLocations.push_back({name, convert(location.pose)});
                    }
                    if(!spaceLocations.empty()) {
                        m_sessionService.m_renderer->set(spaceLocations);
                    }

                    frame = frameLoop({ imageId
                                        , {viewState, std::move(views)}
                                        , m_sessionService.m_baseSpace
                                        , m_frameState.predictedDisplayTime
                                        , m_frameState.predictedDisplayPeriod});

#ifdef USE_MIRROR_WINDOW
                    m_sessionService.m_graphics->mirror(imageId);
#endif

                }
                xrReleaseSwapchainImage(swapchain.handle, nullptr);
                endFrame();
            }

            XrFrameEndInfo endFrameInfo = makeStruct<XrFrameEndInfo>();
            endFrameInfo.environmentBlendMode = frame.blendMode;
            endFrameInfo.layerCount = frame.layers.size();
            endFrameInfo.layers = frame.layers.data();
            endFrameInfo.displayTime = m_frameState.predictedDisplayTime;
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

    void SessionStateRunning::endFrame() {
        m_sessionService.m_renderer->endFrame();
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

    void SessionFocused::processFrame() {
        auto syncInfo = makeStruct<XrActionsSyncInfo>();
        syncInfo.countActiveActionSets = m_sessionService.m_activeActionSets.size();
        syncInfo.activeActionSets = m_sessionService.m_activeActionSets.data();
        CHECK_XR(xrSyncActions(m_sessionService.m_session, &syncInfo));

        const auto& activeSets = m_sessionService.m_activeActionSets;
        const auto& session = m_sessionService.m_session;
        const auto& actionSpaces = m_sessionService.m_actionSpaces;
        const auto& baseSpace = m_sessionService.m_baseSpace;

        m_sessionService.m_renderer->beginFrame();

        for(auto& binding : m_sessionService.m_actionSetBindings) {
            bool inActive = std::any_of(activeSets.begin(), activeSets.end(), [&binding](const auto& aSet){ return aSet.actionSet != binding.xrActionSet; });
            if(inActive) continue;

            for(const auto& [name, action] : binding.actions) {
                assert(binding.actionSet.actions.contains(name));

                auto getInfo = makeStruct<XrActionStateGetInfo>();
                getInfo.action = action._;
                switch(action.type) {
                    case XR_ACTION_TYPE_BOOLEAN_INPUT : {
                        auto state = makeStruct<XrActionStateBoolean>();
                        CHECK_XR(xrGetActionStateBoolean(session, &getInfo, &state));
                        if(state.isActive) {
                            binding.actionSet.actions[name].isActive = true;
                            binding.actionSet.actions[name].changed = state.changedSinceLastSync;
                            binding.actionSet.actions[name]._value = static_cast<bool>(state.currentState);
                        }
                        break;
                    }
                    case XR_ACTION_TYPE_FLOAT_INPUT : {
                        auto state = makeStruct<XrActionStateFloat>();
                        CHECK_XR(xrGetActionStateFloat(session, &getInfo, &state));
                        if(state.isActive) {
                            binding.actionSet.actions[name].isActive = true;
                            binding.actionSet.actions[name].changed = state.changedSinceLastSync;
                            binding.actionSet.actions[name]._value = state.currentState;
                        }
                        break;
                    }
                    case XR_ACTION_TYPE_VECTOR2F_INPUT : {
                        auto state = makeStruct<XrActionStateVector2f>();
                        CHECK_XR(xrGetActionStateVector2f(session, &getInfo, &state));
                        if(state.isActive) {
                            binding.actionSet.actions[name].isActive = true;
                            binding.actionSet.actions[name].changed = state.changedSinceLastSync;
                            binding.actionSet.actions[name]._value = glm::vec2(state.currentState.x, state.currentState.y);
                        }
                        break;
                    }
                    case XR_ACTION_TYPE_POSE_INPUT : {
                        auto state = makeStruct<XrActionStatePose>();
                        CHECK_XR(xrGetActionStatePose(session, &getInfo, &state));
                        if(state.isActive) {
                            binding.actionSet.actions[name].isActive = true;

                            auto space = actionSpaces.at(name);
                            auto location = makeStruct<XrSpaceLocation>();
                            xrLocateSpace(space, baseSpace, m_frameState.predictedDisplayTime, &location);
                            binding.actionSet.actions[name]._value = convert(location.pose);
                        }
                        break;
                    }
                    case XR_ACTION_TYPE_VIBRATION_OUTPUT :
                    case XR_ACTION_TYPE_MAX_ENUM:
                        break; // ignore non input action types

                }
            }
            auto vibrations = m_sessionService.m_renderer->set(binding.actionSet);
            for(const auto& vibration : vibrations) {
                auto action = m_sessionService.m_actionSetBindings.front().actions.at(vibration.action);
                auto info = makeStruct<XrHapticActionInfo>();
                info.action = action._;
                auto hapticVibration = makeStruct<XrHapticVibration>();
                hapticVibration.duration = vibration.duration;
                hapticVibration.frequency = vibration.frequency;
                hapticVibration.amplitude = vibration.amplitude;
                xrApplyHapticFeedback(m_sessionService.m_session, &info, reinterpret_cast<XrHapticBaseHeader*>(&hapticVibration));
            }
        }

        SessionStateRunning::processFrame();
    }

}
