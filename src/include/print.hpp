#pragma once

#include <iostream>
#include <format>
#include <utility>

template<typename... Args>
inline void fmt_print( const std::_Fmt_string<Args...> fmt, Args&&... args) {
    std::cout << std::format(fmt, args...);
}

template<typename... Args>
inline void fmt_error( const std::_Fmt_string<Args...> fmt, Args&&... args) {
    std::cerr << std::format(fmt, args...);
}