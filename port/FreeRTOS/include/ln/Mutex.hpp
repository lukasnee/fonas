/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "ln/MutexI.hpp"
#include "projdefs.h"

#include <FreeRTOS/Mutex.hpp>
#include <FreeRTOS/Kernel.hpp>

namespace ln {

class Mutex : public MutexI {
protected:
    bool ll_lock(const std::chrono::milliseconds &timeout) final {
        return this->mutex.lock(pdMS_TO_TICKS(timeout.count()));
    }

    bool ll_unlock() final { return this->mutex.unlock(); }

    FreeRTOS::StaticMutex mutex;
};

class RecursiveMutex : public MutexI {
protected:
    bool ll_lock(const std::chrono::milliseconds &timeout) final {
        return this->recursive_mutex.lock(pdMS_TO_TICKS(timeout.count()));
    }

    bool ll_unlock() final { return this->recursive_mutex.unlock(); }

    FreeRTOS::StaticRecursiveMutex recursive_mutex;
};

} // namespace ln
