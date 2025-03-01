#pragma once

#include <iostream>
#include <format>
#include <utility>

template <class... _Types>
inline void fmt_print( const std::format_string<_Types...> fmt, _Types&&... args) {
    std::cout << std::format(fmt, args...);
}

template <class... _Types>
inline void fmt_error( const std::format_string<_Types...> fmt, _Types&&... args) {
    std::cerr << std::format(fmt, args...);
}