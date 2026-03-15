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
#include <ctime>
#include <ratio>
#include <utility>

namespace ln {

/**
 * @brief The system uptime clock. The epoch is the system start time, and the
 * time_point is the system uptime.
 *
 * @see<https://en.cppreference.com/w/cpp/named_req/Clock.html>.
 */
struct Clock {
    typedef uint32_t rep;
    typedef std::milli period;
    typedef std::chrono::duration<rep, period> duration;
    typedef std::chrono::time_point<Clock> time_point;

    static const bool is_steady = true;

    static time_point now() noexcept;

    static time_t to_time_t(const time_point &t) noexcept {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   t.time_since_epoch())
            .count();
    }

    static bool to_utc_tm(const time_point &t, std::tm *tm_buf) noexcept {
        time_t tt = to_time_t(t);
        return gmtime_r(&tt, tm_buf) != nullptr;
    }

    static std::tm to_utc_tm(const time_point &t) noexcept {
        std::tm tm_buf;
        to_utc_tm(t, &tm_buf);
        return tm_buf;
    }

    /**
     * @brief Converts a time_point to UTC std::tm and remainder duration.
     *
     * @param t
     * @retval std::pair<std::tm, duration>
     */
    static std::pair<std::tm, duration> to_utc_tm_rem(
        const time_point &t) noexcept {
        return {to_utc_tm(t),
                t.time_since_epoch() -
                    duration_cast<std::chrono::seconds>(t.time_since_epoch())};
    }
};

} // namespace ln
