/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/ln.h"

#include "ln/Clock.hpp"
#include "ln/MutexI.hpp"

#include "FreeRTOS/Kernel.hpp"

namespace ln {

/**
 * @brief Converts a TickType_t to time_point.
 *
 * @note Expressed in similar style as
 * <https://en.cppreference.com/w/cpp/chrono/system_clock/from_time_t.html>.
 *
 * @param t TickType_t value to convert.
 * @return time_point The time_point corresponding to the TickType_t.
 */
static constexpr Clock::time_point from_tick_count(TickType_t t) noexcept {
    return Clock::time_point{std::chrono::milliseconds(
        t * (static_cast<TickType_t>(std::milli::den)) / configTICK_RATE_HZ)};
}

Clock::time_point Clock::now() noexcept {
    return fromTickCount(interrupt_context()
                             ? FreeRTOS::Kernel::getTickCount()
                             : FreeRTOS::Kernel::getTickCountFromISR());
}

static_assert(Clock::duration::max().count() == portMAX_DELAY,
              "Clock::duration::max() must be equivalent "
              "to portMAX_DELAY");

std::chrono::milliseconds MutexI::max_timeout() {
    return Clock::duration::max();
}

std::chrono::milliseconds get_uptime_ms() {
    return duration_cast<std::chrono::milliseconds>(
        Clock::now().time_since_epoch());
}

bool interrupt_context() { return xPortIsInsideInterrupt(); }

std::string_view get_task_name() {
    return (interrupt_context() ? nullptr
                                : pcTaskGetName(xTaskGetCurrentTaskHandle()));
}

} // namespace ln

extern "C" void vApplicationMallocFailedHook(void) { LN_PANIC(); }
