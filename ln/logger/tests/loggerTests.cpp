#include "ln/logger/logger.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Logger: Log levels", "[logger]") {

    auto logger_config = ln::logger::get_instance().get_config();
    constexpr size_t logger_out_buf_size = 512;
    static std::array<char, logger_out_buf_size> logger_out_buf{};
    logger_config.out_buf = logger_out_buf;
    logger_config.eol = "\n";
    logger_config.enabled_run_time = true;
    logger_config.color = true;
    logger_config.log_level = ln::logger::Level::debug;
    ln::logger::get_instance().set_config(logger_config);

    LOG_MODULE(test, ln::logger::Level::notset);

    LOG_DEBUG("This is a debug message {}, {}", 42.37, "hello");
    LOG_INFO("Info with swapped args: {1} then {0}", "zero", "one");
    LOG_WARNING("Named: user={user}, tries={tries}", fmt::arg("user", "alice"),
                fmt::arg("tries", 3));
    using namespace std::chrono_literals;
    auto duration = 1234ms;
    LOG_ERROR("Error after duration: {}", duration);
    LOG_CRITICAL("Aligned: |{:^10}|{:>5}|{:<5}|", "mid", 42, "x");
    LOG_FLUSH();
}
