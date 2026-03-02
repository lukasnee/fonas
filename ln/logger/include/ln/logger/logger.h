/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#ifdef LN_LOGGER

#define LOG_SCOPE(_logger_module)                                              \
    ln::logger::Module *__logger_module_curr_scope [[maybe_unused]] =          \
        &_logger_module

#define LOG_MODULE_DEFINITION(_obj_name, _name, _level)                        \
    ln::logger::Module _obj_name { #_name, _level, ln::logger::get_instance() }

#define LOG_MODULE(_name, _level)                                              \
    static LOG_MODULE_DEFINITION(logger_module, _name, _level);                \
    static LOG_SCOPE(logger_module)

#define LOG_MODULE_CLASS_MEMBER(_name, _level)                                 \
    LOG_MODULE_DEFINITION(logger_module, _name, _level);                       \
    LOG_SCOPE(logger_module)

#define LOG(_level, ...) __logger_module_curr_scope->log(_level, __VA_ARGS__);
#define LOG_DEBUG(...) LOG(ln::logger::Level::debug, __VA_ARGS__)
#define LOG_INFO(...) LOG(ln::logger::Level::info, __VA_ARGS__)
#define LOG_WARNING(...) LOG(ln::logger::Level::warning, __VA_ARGS__)
#define LOG_ERROR(...) LOG(ln::logger::Level::error, __VA_ARGS__)
#define LOG_CRITICAL(...) LOG(ln::logger::Level::critical, __VA_ARGS__)

#define LOG_FLUSH() ln::logger::get_instance().flush_buffer()

#else

#define LOG_SCOPE(_logger_module)
#define LOG_MODULE_DEFINITION(_obj_name, _name, _level)
#define LOG_MODULE(_name, _level)
#define LOG_MODULE_CLASS_MEMBER(_name, _level)

#define LOG(_level, ...)
#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_WARNING(...)
#define LOG_ERROR(...)
#define LOG_CRITICAL(...)

#define LOG_FLUSH()

#endif // LN_LOGGER

#include "ln/File.hpp"
#include "ln/Mutex.hpp"

#include <span>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace ln::logger {

enum Level {
    notset = 0,
    lowest = 1,
    debug = 10,
    info = 20,
    warning = 30,
    error = 40,
    critical = 50,

    _max = critical,
};

struct Config {
    /* Output stream */
    File out_file = File(stdout);
    std::span<char> out_buf = {};
    /* Output buffer flush threshold. If free space is less than this value
    after a log message is written to the buffer, the content will be flushed to
    the out_file. You probably want this size threshold to be of an average log
    message or a bit more (there's a tradeoff between buffer use efficiency and
    risk of overflow and loss of part of the message) */
    static constexpr size_t out_buf_flush_threshold = 128;
    /* Switch logger on/off at compile time */
    static constexpr bool enabled_compile_time = true;
    /* Switch logger on/off at run-time */
    bool enabled_run_time = false;
    /* Global log level printing threshold */
    Level log_level = Level::info;
    /* Colorize log messages */
    bool color = false;
    /* end of line character(s) */
    const char *eol = "\n";
    /* Print log message header */
    bool print_header_enabled = true;
};

class Module;

/**
 * @brief RTOS logger with Python logging style.
 */
class Logger {
public:
    static Logger &get_instance();

    void enable();

    void set_level(Level log_level);

    bool set_config(const Config &config);

    const Config &get_config() const { return config; }

    int log(const Module &module, const Level &level, const char *fmt,
            va_list &arg_list);

    /**
     * @brief Flush the output buffer to the output stream. Note that buffer
     * is flushed automatically when it reaches the
     * Config::out_buffer_auto_flush_threshold size.
     *
     * This function is thread-safe and cannot be called from an ISR
     * context.
     */
    void flush_buffer();

protected:
    Config config = {};

    friend class Module;

private:
    Logger() = default;
    Logger(Logger const &) = delete;
    void operator=(Logger const &) = delete;
    ~Logger() = default;

    int _log(const Module &module, const Level &level, const char *fmt,
             va_list &arg_list);
    void _flush_buffer();

    int print_header(const Module &module, const Level &level);
    int printf_buf(const char *fmt, ...);
    int vprintf_buf(const char *fmt, va_list &args);

    RecursiveMutex mutex;

    size_t out_buf_len = 0;
};

/**
 * @brief A shorthand for logger::Logger::get_instance().
 *
 * @retval Logger&
 */
Logger &get_instance();

class Module {
public:
    explicit Module(std::string_view name, Level log_level = Level::notset,
                    Logger &logger = Logger::get_instance());

    template <typename... Args> void debug(const char *fmt, Args &&...args) {
        this->log(Level::debug, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args> void info(const char *fmt, Args &&...args) {
        this->log(Level::info, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args> void warning(const char *fmt, Args &&...args) {
        this->log(Level::warning, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args> void error(const char *fmt, Args &&...args) {
        this->log(Level::error, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args> void critical(const char *fmt, Args &&...args) {
        this->log(Level::critical, fmt, std::forward<Args>(args)...);
    }

    void log(const Level &level, const char *fmt, ...);

    void set_level(Level log_level);

private:
    friend class Logger;

    std::string_view name;
    Level log_level;
    Logger &logger;
};

} // namespace ln::logger
