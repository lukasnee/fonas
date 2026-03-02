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

#include <chrono>
#include <mutex>

namespace ln {

class Mutex : public MutexI {
public:
private:
    bool lock(const ::std::chrono::milliseconds &timeout) final {
        return this->mutex.try_lock_for(timeout);
    }
    bool unlock() final {
        this->mutex.unlock();
        return true;
    }

    std::timed_mutex mutex;
};

class RecursiveMutex : public RecursiveMutexI {
public:
private:
    bool lock(const ::std::chrono::milliseconds &timeout) final {
        return this->recursive_mutex.try_lock_for(timeout);
    }
    bool unlock() final {
        this->recursive_mutex.unlock();
        return true;
    }

    std::recursive_timed_mutex recursive_mutex;
};

} // namespace ln
