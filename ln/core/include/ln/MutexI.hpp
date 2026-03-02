/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <chrono>

namespace ln {

class MutexBase {
public:
    virtual ~MutexBase() = default;

    /**
     * @brief Returns the maximum timeout that can be used in the lock()
     * function.
     *
     * @attention Port specific implementation.
     *
     * @retval std::chrono::milliseconds
     */
    static std::chrono::milliseconds max_timeout();

    template <typename Rep, typename Period>
    bool lock(
        const std::chrono::duration<Rep, Period> &timeout = max_timeout()) {
        return this->lock(
            std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
    }
    virtual bool unlock() = 0;
    virtual bool lock(const std::chrono::milliseconds &timeout) = 0;
};

class MutexI : private MutexBase {
public:
    virtual ~MutexI() = default;

private:
    friend class Mutex;
};

class RecursiveMutexI : private MutexBase {
public:
    virtual ~RecursiveMutexI() = default;

private:
    friend class RecursiveMutex;
};

class LockGuard {
public:
    explicit LockGuard(MutexBase &mutex) : mutex(mutex) {
        this->mutex.lock(MutexBase::max_timeout());
    }

    ~LockGuard() { this->mutex.unlock(); }

private:
    MutexBase &mutex;
};

} // namespace ln
