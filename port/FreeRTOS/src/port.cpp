/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/ln.h"

#include "FreeRTOS/Addons/Clock.hpp"
#include "FreeRTOS/Addons/Kernel.hpp"

namespace ln {

std::chrono::milliseconds get_uptime_ms() {
    return duration_cast<std::chrono::milliseconds>(
        FreeRTOS::Addons::Clock::now().time_since_epoch());
}

bool interrupt_context() {
    return FreeRTOS::Addons::Kernel::isInsideInterrupt();
}

} // namespace ln

extern "C" void vApplicationMallocFailedHook(void) { LN_PANIC(); }
