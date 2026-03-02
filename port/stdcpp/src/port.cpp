/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/MutexI.hpp"

#include <chrono>
#include <cstdlib>
#include <cstdio>

namespace ln {

std::chrono::milliseconds MutexBase::max_timeout() {
    return std::chrono::milliseconds::max();
}

void panic(const char *file, int line, const char *message) {
    printf("\n%s:%d: panic%s%s\n", file, line, (message ? ": " : "!"),
           (message ? message : ""));
    std::abort();
}

void reset() { std::abort(); }

const auto g_start_time = std::chrono::steady_clock::now();

std::chrono::milliseconds get_uptime_ms() {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = now - g_start_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
}

} // namespace ln
