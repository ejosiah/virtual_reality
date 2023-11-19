#pragma once

#include "xr_struct_mapping.hpp"

#include <openxr/openxr.h>

#include <vector>
#include <type_traits>

namespace vr {
    template<typename T>
    std::tuple<XrResult, std::vector<T>> enumerate(XrInstance instance, XrSystemId system, auto&& enumerator) {
        uint32_t size{0};
        T zero{};
        if constexpr (std::is_class_v<T>) {
            zero = makeStruct<T>();
        }
        enumerator(instance, system, 0, &size, &zero);

        std::vector<T> list(size, zero);

        XrResult result = enumerator(instance, system, size, &size, list.data());

        return std::make_tuple(result, list);
    }

    template<typename T>
    std::tuple<XrResult, std::vector<T>> enumerate(XrSession session, auto&& enumerator) {
        uint32_t size{0};
        T zero{};
        if constexpr (std::is_class_v<T>) {
            zero = makeStruct<T>();
        }
        enumerator(session, 0, &size, &zero);

        std::vector<T> list(size, zero);

        XrResult result = enumerator(session, size, &size, list.data());

        return std::make_tuple(result, list);
    }

    template<typename T>
    std::tuple<XrResult, std::vector<T>> enumerate(auto&& enumerator) {
        uint32_t size{0};
        T zero{};
        if constexpr (std::is_class_v<T>) {
            zero = makeStruct<T>();
        }
        enumerator(&size, &zero);

        std::vector<T> list(size, zero);

        XrResult result = enumerator(&size, list.data());

        return std::make_tuple(result, list);
    }
}