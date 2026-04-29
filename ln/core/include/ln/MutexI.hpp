// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <chrono>

namespace ln {

class MutexI {
public:
    virtual ~MutexI() = default;

    /**
     * @brief Returns the maximum timeout that can be used in the lock()
     * function.
     *
     * @attention Port specific implementation.
     *
     * @retval std::chrono::milliseconds
     */
    static std::chrono::milliseconds max_timeout();

    bool lock() { return this->ll_lock(max_timeout()); }

    template <typename Rep, typename Period>
    bool lock(const std::chrono::duration<Rep, Period> &timeout) {
        return this->ll_lock(
            std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
    }

    bool unlock() { return this->ll_unlock(); }

protected:
    virtual bool ll_lock(const std::chrono::milliseconds &timeout) = 0;
    virtual bool ll_unlock() = 0;
};

class Mutex; // Port specific implementation of MutexI

class RecursiveMutex; // Port specific implementation of MutexI

class LockGuard {
public:
    explicit LockGuard(MutexI &mutex) : mutex(mutex) { this->mutex.lock(); }

    ~LockGuard() { this->mutex.unlock(); }

private:
    MutexI &mutex;
};

} // namespace ln
