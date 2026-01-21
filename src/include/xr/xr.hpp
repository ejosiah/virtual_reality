#pragma once

#include <check.hpp>
#include <xr_struct_mapping.hpp>

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <vector>

namespace xr {

    template<typename T, typename Fn, typename... Args>
    std::tuple<XrResult, std::vector<T>> callEnumerator(Fn &&fn, Args &&... args) {
        auto count = 0u;
        auto res = std::forward<Fn>(fn)(std::forward<Args>(args)..., 0, &count, nullptr);

        if (res != XR_SUCCESS) return std::make_tuple(res, std::vector<T>{});

        std::vector<T> result(count, makeStruct<T>());

        res = std::forward<Fn>(fn)(std::forward<Args>(args)..., count, &count, result.data());

        if (res != XR_SUCCESS) return std::make_tuple(res, std::vector<T>{});

        result.resize(count);
        return std::make_tuple(res, result);
    }

    template<typename T, typename Fn, typename... Args>
    std::tuple<XrResult, T> callGetter(Fn &&fn, Args &&... args) {
        T t = makeStruct<T>();
        auto res = std::forward<Fn>(fn)(std::forward<Args>(args)..., &t);
        return std::make_tuple(res, t);
    }

    inline std::vector<XrView> locateViews(XrSession session,const XrViewLocateInfo &locateInfo, XrViewState &viewState, XrInstance instance = XR_NULL_HANDLE) {
        auto [res, views] = callEnumerator<XrView>(xrLocateViews, session, &locateInfo, &viewState);

        if(instance) {
            LOG_ERROR(instance, res)
        }
        CHECK_XR(res);
        return views;
    }

    inline std::vector<XrViewConfigurationView> enumerateViewConfigurationViews(XrInstance instance, XrSystemId systemId
                                                                         , XrViewConfigurationType viewConfigurationType) {
        auto [res, views] = callEnumerator<XrViewConfigurationView>(
                xrEnumerateViewConfigurationViews,
                instance,
                systemId,
                viewConfigurationType
        );
        CHECK_XR(res);
        return views;
    }

     inline XrSpaceLocation locateSpace( XrSpace space, XrSpace baseSpace, XrTime time) {
        auto [res, location] = callGetter<XrSpaceLocation>(xrLocateSpace, space, baseSpace, time);
         CHECK_XR(res);
         return location;

    }

}