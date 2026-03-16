#include "ln/logger/logger.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>

TEST_CASE("Logger: Log levels", "[logger]") {

    constexpr size_t logger_out_buf_size = 512;
    static std::array<char, logger_out_buf_size> logger_out_buf{};
    logger_out_buf.fill('\0');

    constexpr size_t capture_buf_size = 1024;
    static std::array<char, capture_buf_size> capture_buf{};
    capture_buf.fill('\0');
    ln::File capture_file(std::span<char>(capture_buf), "w");

    auto logger_config = ln::logger::get_instance().get_config();
    logger_config.out_file = capture_file;
    logger_config.out_buf = logger_out_buf;
    logger_config.eol = "\n";
    logger_config.enabled_run_time = true;
    logger_config.color = false;
    logger_config.print_header_enabled = false;
    logger_config.log_level = ln::logger::Level::debug;
    ln::logger::get_instance().set_config(logger_config);

    LOG_MODULE(test, ln::logger::Level::notset);

    LOG_DEBUG("This is a debug message {}, {}", 42.37, "hello");
    LOG_INFO("Info with swapped args: {} then {}", "zero", "one");
    LOG_WARNING("Named: user={}, tries={}", "alice", 3);
    using namespace std::chrono_literals;
    LOG_ERROR("Error after duration: {}", 1234ms);
    LOG_CRITICAL("Aligned: |{:>10}|{:5}|{:<5}|", "mid", 42, "x");
    LOG_FLUSH();
    std::fflush(capture_file.c_file());
    const std::string output(capture_buf.data());
    const std::string expected = "This is a debug message 42.37, hello\n"
                                 "Info with swapped args: zero then one\n"
                                 "Named: user=alice, tries=3\n"
                                 "Error after duration: 1234ms\n"
                                 "Aligned: |       mid|   42|x    |\n";

    CAPTURE(output);
    CAPTURE(expected);
    REQUIRE(output == expected);
}
