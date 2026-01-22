#pragma once

#include <vector>
#include <ranges>
#include <algorithm>

template <typename R, typename Func>
auto map_range(R&& range, Func&& func) {
    auto view = std::forward<R>(range) | std::views::transform(std::forward<Func>(func));
    return std::vector(view.begin(), view.end());
}

template <typename Range, typename Func>
auto flat_map_range(Range&& range, Func&& func) {
    auto joined =
            std::forward<Range>(range)
            | std::views::transform(std::forward<Func>(func))
            | std::views::join;

    using value_t = std::ranges::range_value_t<decltype(joined)>;
    return std::vector<value_t>(std::ranges::begin(joined), std::ranges::end(joined));
}

template<typename T>
std::vector<T> unique(std::vector<T> v)
{
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
    return v;
}