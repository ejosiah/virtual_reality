#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vr {

    template<typename T>
    std::tuple<VkResult, std::vector<T>> enumerate(VkInstance instance, auto&& enumerator) {
        uint32_t size{0};
        T zero{};

        enumerator(instance, system, &size, &zero);

        std::vector<T> list(size);

        VkResult result = enumerator(instance, &size, list.data());

        return std::make_tuple(result, list);
    }

    template<typename T>
    std::vector<T> get(VkPhysicalDevice physicalDevice, auto&& getter) {
        uint32_t size{0};
        T zero{};

        getter(physicalDevice, &size, &zero);

        std::vector<T> list(size, zero);

        getter(physicalDevice, &size, list.data());

        return list;
    }
}
