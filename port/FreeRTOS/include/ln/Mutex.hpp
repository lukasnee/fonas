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

#include <FreeRTOS/Mutex.hpp>
#include <FreeRTOS/Addons/Clock.hpp>

namespace ln {

static std::chrono::milliseconds max_timeout() {
    return FreeRTOS::Addons::Clock::duration::max();
}

class Mutex : public MutexI {
public:
private:
    bool lock(const std::chrono::milliseconds &timeout) final {
        return this->mutex.take(timeout);
    }
    bool unlock() final { return this->mutex.give(); }

    FreeRTOS::StaticMutex mutex;
};

class RecursiveMutex : public RecursiveMutexI {
public:
private:
    bool lock(const std::chrono::milliseconds &timeout) final {
        return this->recursive_mutex.take(timeout);
    }
    bool unlock() final { return this->recursive_mutex.give(); }

    FreeRTOS::StaticRecursiveMutex recursive_mutex;
};

} // namespace ln
