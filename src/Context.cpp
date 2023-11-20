#include "check.hpp"
#include "xr_struct_mapping.hpp"
#include "vr/Context.hpp"
#include "vr/Enumerators.hpp"
#include "vr/Creation.hpp"

#ifndef NDEBUG
#ifdef XR_DEBUG
#include "vr/XrDebuging.hpp"
#endif
#endif

#include <openxr_ext_loader.h>

namespace vr {

    void log(const auto& ctx) {
        auto instanceProps = makeStruct<XrInstanceProperties>();
        xrGetInstanceProperties(ctx.instance, &instanceProps);

        auto systemProps = makeStruct<XrSystemProperties>();
        xrGetSystemProperties(ctx.instance, ctx.systemId, &systemProps);

        std::stringstream ss;
        ss << "\nOpenXR instance successfully created\n";
        ss << std::format("Runtime: {}, version: {}.{}.{}\n"
                , instanceProps.runtimeName
                , XR_VERSION_MAJOR(instanceProps.runtimeVersion)
                , XR_VERSION_MINOR(instanceProps.runtimeVersion)
                , XR_VERSION_PATCH(instanceProps.runtimeVersion));

        ss << "System info:\n";
        ss << std::format("\tName: {}\n", systemProps.systemName);
        ss << "\tGraphics properties:\n";
        ss
                << std::format("\t\tMax swapchain width: {}\n", systemProps.graphicsProperties.maxSwapchainImageWidth);
        ss << std::format("\t\tMax swapchain height: {}\n",
                          systemProps.graphicsProperties.maxSwapchainImageHeight);
        ss << std::format("\t\tMax layer count: {}\n", systemProps.graphicsProperties.maxLayerCount);
        ss << "\tTracking properties:\n";
        ss << std::format("\t\t Position tracking {}supported\n",
                          systemProps.trackingProperties.positionTracking ? "" : "Un");
        ss << std::format("\t\t Orientation tracking {}supported\n",
                          systemProps.trackingProperties.orientationTracking ? "" : "Un");

        spdlog::info("{}", ss.str());
    }

    std::optional<Context> createContext(const ContextCreation& creation) {

        auto createInfo = makeStruct<XrInstanceCreateInfo>();

#ifndef NDEBUG
#ifdef XR_DEBUG

     auto debugCreateInfo = vr::xrDebugCreateInfo();
     createInfo.next = &debugCreateInfo;
#endif
#endif

        std::vector<const char*> extensions = creation.extensions;
        extensions.push_back(creation.graphicsExtension());

        strcpy_s(createInfo.applicationInfo.applicationName, creation._appName.c_str());
        createInfo.applicationInfo.applicationVersion = creation._appVersion;

        strcpy_s(createInfo.applicationInfo.engineName, creation._engineName.c_str());

        createInfo.applicationInfo.engineVersion = creation._engineVersion;
        createInfo.applicationInfo.apiVersion = creation._apiVersion;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.enabledExtensionNames = extensions.data();

        Context ctx{};

        if(XR_FAILED(xrCreateInstance(&createInfo, &ctx.instance))) {
            return std::nullopt;
        }

        auto getInfo = makeStruct<XrSystemGetInfo>();
        getInfo.formFactor = creation._formFactor;  // TODO check form factor is supported

        if(XR_FAILED(xrGetSystem(ctx.instance, &getInfo, &ctx.systemId))) {
            return std::nullopt;
        }

        log(ctx);

        initExtLoader(ctx.instance);

#ifndef NDEBUG
#ifdef XR_DEBUG
        xrCreateDebugUtilsMessengerEXT(ctx.instance, &debugCreateInfo, &ctx.debugMessenger);
#endif
#endif

        ctx.graphicsContext = creation.graphics().create(ctx);

        return ctx;
    }

    [[nodiscard]]
    bool Context::isSupported(XrViewConfigurationType type) const {
        assert(instance != XR_NULL_HANDLE);
        assert(systemId != XR_NULL_SYSTEM_ID);

        auto [result, viewTypes] = enumerate<XrViewConfigurationType>(instance, systemId, xrEnumerateViewConfigurations);
        CHECK_XR(result);

        return
                std::any_of(viewTypes.begin(), viewTypes.end(), [type](const auto viewType){ return viewType == type; });
    }

    std::vector<XrViewConfigurationView> Context::views(XrViewConfigurationType viewType) const {
        auto [result, views] = enumerate<XrViewConfigurationView>([&](auto size, auto ptr) {
            return xrEnumerateViewConfigurationViews(instance, systemId, viewType, *size, size, ptr);
        });
        CHECK_XR(result);
        return views;
    }

    void ContextCreation::validate() const {
        if(_appName.empty()) {
            throw std::runtime_error{"ContextCreation: appName is required"};
        }
        if(_apiVersion == 0) {
            throw std::runtime_error("ContextCreation: apiVersion is required");
        }
    }

    void Context::destroy() {
        spdlog::debug("destroying OpenXR context ...");
#ifndef NDEBUG
#ifdef XR_DEBUG
        xrDestroyDebugUtilsMessengerEXT(debugMessenger);
#endif
#endif
        xrDestroyInstance(instance);
        spdlog::info("OpenXR context destroyed");
        instance = XR_NULL_HANDLE;
        systemId = 0;
    }

    std::optional<Context> ContextCreation::create() const {
        validate();
        return createContext(*this);
    }
}