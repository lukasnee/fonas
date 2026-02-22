/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/logger/logger.h"
#include "ln/ln.h"

#include <cstdio>

namespace ln::logger {

Logger &Logger::get_instance() {
    static Logger instance;
    return instance;
};

Logger &get_instance() { return Logger::get_instance(); }

void Logger::enable() {
    auto config = this->config;
    config.enabled_run_time = true;
    this->set_config(config);
}

void Logger::flush_buffer() {
    if constexpr (!Config::enabled_compile_time) {
        return;
    }
    if (FreeRTOS::Addons::Kernel::isInsideInterrupt()) {
        LN_PANIC();
    }
    FreeRTOS::Addons::LockGuard lock_guard(this->mutex);
    if (!this->config.enabled_run_time) {
        return;
    }
    this->_flush_buffer();
}

void Logger::set_level(Level log_level) { this->config.log_level = log_level; }

bool Logger::set_config(const Config &config) {
    // TODO: validation here
    this->config = config;
    return true;
}

Module::Module(const std::string_view name, Level log_level, Logger &logger)
    : name(name.data()), log_level(log_level), logger(logger) {}

void Module::log(const Level &level, const char *fmt, ...) {
    if constexpr (!Config::enabled_compile_time) {
        return;
    }
    if (!this->logger.config.enabled_run_time) {
        return;
    }
    if (level < (this->log_level == Level::notset
                     ? this->logger.config.log_level
                     : this->log_level)) {
        return;
    }
    va_list arg_list;
    va_start(arg_list, fmt);
    this->logger.log(*this, level, fmt, arg_list);
    va_end(arg_list);
}

void Module::set_level(Level log_level) { this->log_level = log_level; }

int Logger::log(const Module &module, const Level &level, const char *fmt,
                va_list &arg_list) {
    const auto is_interrupt_context =
        FreeRTOS::Addons::Kernel::isInsideInterrupt();
    if (!is_interrupt_context && !this->mutex.lock()) {
        return 0;
    }
    const auto rc = this->_log(module, level, fmt, arg_list);
    if (!is_interrupt_context) {
        this->mutex.unlock();
    }
    return rc;
}

int Logger::_log(const Module &module, const Level &level, const char *fmt,
                 va_list &arg_list) {
    int chars_printed = 0;
    if (this->config.print_header_enabled) {
        LN_CHECK(this->print_header(module, level), rc, rc < 0,
                 { chars_printed += rc; }, {});
    }
    LN_CHECK(this->vprintf_buf(fmt, arg_list), rc, rc < 0,
             { chars_printed += rc; }, {});
    LN_CHECK(this->printf_buf("%s", this->config.eol), rc, rc < 0,
             { chars_printed += rc; }, {});
    return chars_printed;
}

int Logger::print_header(const Module &module, const Level &level) {

#define ANSI_COLOR_BLACK "\e[30m"
#define ANSI_COLOR_RED "\e[31m"
#define ANSI_COLOR_GREEN "\e[32m"
#define ANSI_COLOR_YELLOW "\e[33m"
#define ANSI_COLOR_BLUE "\e[34m"
#define ANSI_COLOR_MAGENTA "\e[35m"
#define ANSI_COLOR_CYAN "\e[36m"
#define ANSI_COLOR_WHITE "\e[37m"
#define ANSI_COLOR_DEFAULT "\e[39m"
#define ANSI_COLOR_RESET "\e[0m"

    struct LevelDescr {
        std::string_view tag_name;
        std::string_view color;
    };
    static constexpr LevelDescr level_descrs[5] = {{"DBG", ANSI_COLOR_MAGENTA},
                                                   {"INF", ANSI_COLOR_DEFAULT},
                                                   {"WRN", ANSI_COLOR_YELLOW},
                                                   {"ERR", ANSI_COLOR_RED},
                                                   {"CRT", ANSI_COLOR_RED}};
    const auto level_clamped = std::min(level, Level::_max);
    const auto level_descr_idx = level_clamped == 0 ? 0 : ((level - 1) / 10);
    using Clock = FreeRTOS::Addons::Clock;
    const auto [tm_buf, sec_remainder] = Clock::to_utc_tm_rem(Clock::now());
    char datetime_buffer[sizeof("YYYY-MM-DD HH:MM:SS")];
    const auto ms = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(sec_remainder)
            .count());
    std::strftime(datetime_buffer, sizeof(datetime_buffer), "%Y-%m-%d %H:%M:%S",
                  &tm_buf);
    const auto current_task_name =
        FreeRTOS::Addons::Kernel::getCurrentTaskName();
    return this->printf_buf(
        "%s.%03lu|%s%s%s|%s%s|%s|", datetime_buffer, ms,
        (this->config.color ? level_descrs[level_descr_idx].color.data() : ""),
        level_descrs[level_descr_idx].tag_name.data(),
        (this->config.color ? ANSI_COLOR_DEFAULT : ""),
        (FreeRTOS::Addons::Kernel::isInsideInterrupt() ? "ISR!" : ""),
        (current_task_name.empty() ? "-" : current_task_name.data()),
        module.name.data());
}

int Logger::printf_buf(const char *fmt, ...) {
    va_list arg_list;
    va_start(arg_list, fmt);
    const auto rc = this->vprintf_buf(fmt, arg_list);
    va_end(arg_list);
    return rc;
}

int Logger::vprintf_buf(const char *fmt, va_list &args) {
    const auto rc =
        vsnprintf(this->config.out_buf.data() + this->out_buf_len,
                  this->config.out_buf.size() - this->out_buf_len, fmt, args);
    if (rc <= 0) {
        return rc;
    }
    this->out_buf_len += static_cast<size_t>(rc);
    if (this->config.out_buf.size() - this->out_buf_len <
        Config::out_buf_flush_threshold) {
        this->_flush_buffer();
    }
    return rc;
}

void Logger::_flush_buffer() {
    if (this->out_buf_len == 0) {
        return;
    }
    std::fwrite(this->config.out_buf.data(), sizeof(char), this->out_buf_len,
                this->config.out_file.c_file());
    this->out_buf_len = 0;
}

} // namespace ln::logger
