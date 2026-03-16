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
#include "ln/Clock.hpp"

#include <fmt/core.h>
#include <fmt/chrono.h>

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
    if (ln::interrupt_context()) {
        LN_PANIC();
    }
    LockGuard lock_guard(this->mutex);
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

void Module::set_level(Level log_level) { this->log_level = log_level; }

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
    const auto now = Clock::now();
    const auto now_as_std_system_clock = std::chrono::system_clock::time_point{
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            now.time_since_epoch())};
    const auto current_task_name = ln::get_task_name();
    using namespace std::string_view_literals;
    return this->print_buf(
        "{:%Y-%m-%d %H:%M:%S}|{}{}{}|{}{}|{}|", now_as_std_system_clock,
        (this->config.color ? level_descrs[level_descr_idx].color : ""sv),
        level_descrs[level_descr_idx].tag_name,
        (this->config.color ? ANSI_COLOR_DEFAULT : ""sv),
        (ln::interrupt_context() ? "ISR!" : ""),
        (current_task_name.empty() ? "-" : current_task_name), module.name);
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
