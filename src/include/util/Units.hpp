#pragma once

#include <cinttypes>
#include <ratio>

namespace units {

    using byte = std::ratio<1, 1>;
    using bytes = std::ratio<1, 1>;

    using kb = std::ratio<1 << 10, 1>;
    using kilobyte = kb;
    using kilobytes = kb;

    using mb = std::ratio<2 << 20, 1>;
    using megabyte = mb;
    using megabytes = mb;

    using gb = std::ratio<2 << 30, 1>;
    using gigabyte = gb;
    using gigabytes = gb;

    inline namespace unit_literals {

        constexpr uint64_t operator""_b(unsigned  long long value) noexcept {
            return value;
        }

        constexpr uint64_t operator""_kb(unsigned  long long value) noexcept {
            return value * kilobyte::num;
        }

        constexpr uint64_t operator""_mb(unsigned  long long value) noexcept {
            return value * megabyte::num;
        }

        constexpr uint64_t operator""_gb(unsigned  long long value) noexcept {
            return value * gigabyte::num;
        }
    }
}