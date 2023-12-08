#pragma once

#include <vector>

template<typename T>
inline T& push_use(std::vector<T>& vector) {
    vector.push_back({});
    return vector.back();
}