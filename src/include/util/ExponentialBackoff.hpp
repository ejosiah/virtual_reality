#pragma once


#include <spdlog/spdlog.h>

#include <cinttypes>
#include <string>
#include <chrono>
#include <format>
#include <utility>

namespace util {

    struct Backoff {

        explicit Backoff(std::string msg) : message(std::move(msg)) {}

        void operator()() {
            static constexpr float MaxSleepTime = 30.f;
            retries++;
            if(retries > maxRetries) {
                spdlog::error("{} after {} retries", message, maxRetries);
                throw cpptrace::runtime_error{std::format("failed to create XR instance after {} retries", maxRetries)};
            }
            auto x = std::roundf(std::expf(0.68f * float(retries)));
            int sleepTime =  static_cast<int>(x);
            spdlog::info("{}, will try again in {} seconds", message, sleepTime);
            std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
        }

        void reset() {
            retries = 0;
        }

    private:
        std::string message{};
        uint32_t maxRetries{5};
        uint32_t retries{};
    };
}