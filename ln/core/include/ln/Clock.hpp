// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <fmt/format.h>

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

template <>
struct fmt::formatter<ln::Clock::duration> : fmt::formatter<uint32_t> {
    auto format(const ln::Clock::duration &d, fmt::format_context &ctx) const {
        const auto ms_total = d.count();
        const auto ms = ms_total % 1000U;
        const auto secs_total = ms_total / 1000U;
        const auto secs = secs_total % 60;
        const auto mins_total = secs_total / 60;
        const auto mins = mins_total % 60;
        const auto hours_total = mins_total / 60;
        const auto hours = hours_total % 24;
        const auto days = hours_total / 24;
        return fmt::format_to(ctx.out(), "{:03d}:{:02d}:{:02d}:{:02d}.{:03d}",
                              days, hours, mins, secs, ms);
    }
};

template <>
struct fmt::formatter<ln::Clock::time_point> : fmt::formatter<uint32_t> {
    auto format(const ln::Clock::time_point &tp,
                fmt::format_context &ctx) const {
        return fmt::format_to(ctx.out(), "{}", tp.time_since_epoch());
    }
};
