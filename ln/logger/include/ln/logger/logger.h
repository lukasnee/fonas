/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef LN_LOGGER

    typedef enum {
        LOGGER_LEVEL_NOTSET = 0,
        LOGGER_LEVEL_LOWEST = 1,
        LOGGER_LEVEL_DEBUG3 = 8,
        LOGGER_LEVEL_DEBUG2 = 9,
        LOGGER_LEVEL_DEBUG = 10,
        LOGGER_LEVEL_INFO = 20,
        LOGGER_LEVEL_WARNING = 30,
        LOGGER_LEVEL_ERROR = 40,
        LOGGER_LEVEL_CRITICAL = 50,

        LOGGER_LEVEL_MAX_ = LOGGER_LEVEL_CRITICAL,
    } LoggerLevel;

    typedef struct {
        const char *name;
        LoggerLevel log_level;
    } LoggerModule;

    void ln_logger_enable();

    void ln_logger_log(LoggerModule *module, LoggerLevel level, const char *fmt,
                       ...) __attribute__((__format__(__printf__, 3, 4)));

    void ln_logger_flush_buffer();

#define LOG_SCOPE(_logger_module)                                              \
    LoggerModule *__logger_curr_scope __attribute__((unused)) = &_logger_module

#define LOG_MODULE_DEFINITION(_obj_name, _name, _level)                        \
    LoggerModule _obj_name = {.name = #_name, .log_level = _level}

#define LOG_MODULE_EXT(_obj_name)                                              \
    extern LoggerModule _obj_name;                                             \
    LOG_SCOPE(_obj_name)

#define LOG_MODULE(_name, _level)                                              \
    static LOG_MODULE_DEFINITION(logger_module, _name, _level);                \
    static LOG_SCOPE(logger_module)

#define LOG_MODULE_CLASS_MEMBER(_name, _level)                                 \
    LOG_MODULE_DEFINITION(logger_module, _name, _level);                       \
    LOG_SCOPE(logger_module)

#define LOG(_level, ...)                                                       \
    ln_logger_log(__logger_curr_scope, _level, __VA_ARGS__);
#define LOG_DEBUG(...) LOG(LOGGER_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) LOG(LOGGER_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARNING(...) LOG(LOGGER_LEVEL_WARNING, __VA_ARGS__)
#define LOG_ERROR(...) LOG(LOGGER_LEVEL_ERROR, __VA_ARGS__)
#define LOG_CRITICAL(...) LOG(LOGGER_LEVEL_CRITICAL, __VA_ARGS__)

#define LOG_FLUSH() ln_logger_flush_buffer()

#else

#define LOG_SCOPE(_logger_module)
#define LOG_MODULE_DEFINITION(_obj_name, _name, _level)
#define LOG_MODULE_EXT(_obj_name)
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

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include "ln/File.hpp"

#include "FreeRTOS/Mutex.hpp"

#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace ln::logger {

using Level = LoggerLevel;

struct Config {
    /* Output stream */
    File out_file = File(stdout);
    /* Output buffer size */
    static constexpr size_t out_buffer_size = 1024;
    /* Output buffer flush threshold */
    static constexpr size_t out_buffer_auto_flush_threshold =
        out_buffer_size / 2;
    /* Switch logger on/off at compile time */
    static constexpr bool enabled_compile_time = true;
    /* Switch logger on/off at run-time */
    bool enabled_run_time = false;
    /* Global log level printing threshold */
    Level log_level = LOGGER_LEVEL_INFO;
    /* Colorize log messages */
    bool color = false;
    /* end of line character(s) */
    const char *eol = "\n";
    /* Print log message header */
    bool print_header_enabled = true;

    static_assert(
        out_buffer_auto_flush_threshold < out_buffer_size,
        "Output buffer flush threshold must be less than output buffer size");
};

class Module : public LoggerModule {
public:
    explicit Module(std::string_view name,
                    Level log_level = LOGGER_LEVEL_NOTSET);

    template <typename... Args>
    void debug(const std::string_view fmt, Args &&...args) {
        log(LOGGER_LEVEL_DEBUG, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void info(const std::string_view fmt, Args &&...args) {
        log(LOGGER_LEVEL_INFO, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void warning(const std::string_view fmt, Args &&...args) {
        log(LOGGER_LEVEL_WARNING, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void error(const std::string_view fmt, Args &&...args) {
        log(LOGGER_LEVEL_ERROR, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void critical(const std::string_view fmt, Args &&...args) {
        log(LOGGER_LEVEL_CRITICAL, fmt, std::forward<Args>(args)...);
    }
    void log(const Level &level, std::string_view fmt, ...);

    void set_level(Level log_level);
};

/**
 * @brief RTOS logger with Python logging style.
 */
class Logger {
public:
    using Level = LoggerLevel;

    static Logger &get_instance();

    void enable();

    static bool is_enabled();

    void set_level(Level log_level);

    bool set_config(const Config &config);

    const Config &get_config() const { return config; }

    int log(const LoggerModule &module, const Level &level,
            std::string_view fmt, const va_list &arg_list);

    /**
     * @brief Flush the output buffer to the output stream. Note that buffer is
     * flushed automatically when it reaches the
     * Config::out_buffer_auto_flush_threshold size.
     *
     * This function is thread-safe and cannot be called from an ISR context.
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

    int log_unsafe(const LoggerModule &module, const Level &level,
                   std::string_view fmt, const va_list &arg_list);

    void clear_buffer_unsafe();
    void flush_buffer_unsafe();

    int print_header(File &file, const LoggerModule &module,
                     const Level &level) const;
    static int printf(File &file, const char *fmt, ...);

    FreeRTOS::StaticRecursiveMutex mutex;

    std::array<char, Config::out_buffer_size> buff_mem{};
};

/**
 * @brief A shorthand for logger::Logger::get_instance().
 *
 * @retval Logger&
 */
Logger &get_instance();

} // namespace ln::logger

#endif // __cplusplus
