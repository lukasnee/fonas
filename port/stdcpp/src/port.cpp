/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/MutexI.hpp"
#include "ln/Clock.hpp"

#include <chrono>
#include <cstdlib>
#include <cstdio>

namespace ln {

const auto g_start_time = std::chrono::steady_clock::now();

Clock::time_point Clock::now() noexcept {
    return Clock::time_point{std::chrono::duration_cast<Clock::duration>(
        std::chrono::steady_clock::now() - g_start_time)};
}

std::chrono::milliseconds MutexI::max_timeout() {
    return std::chrono::milliseconds::max();
}

void panic(const char *file, int line, const char *message) {
    printf("\n%s:%d: panic%s%s\n", file, line, (message ? ": " : "!"),
           (message ? message : ""));
    std::abort();
}

void reset() { std::abort(); }

std::chrono::milliseconds get_uptime_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now().time_since_epoch());
}

bool interrupt_context() { return false; }

std::string_view get_task_name() { return {}; }

} // namespace ln
