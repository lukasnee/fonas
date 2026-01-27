/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "ln/File.hpp"
#include "ln/Interpreter.hpp"
#include "ln/shell/History.hpp"
#include "ln/shell/Input.hpp"
#include "ln/shell/Cmd.hpp"

#include "ln/logger/logger.h"

#include <array>
#include <span>
#include <cstdarg>
#include <cstring>
#include <string_view>
#include <tuple>

// TODO: extract color code for logger and CLI
// TODO: rework the OK FAIL tags, color codes, etc. Make the interface cleaner
// and flexible.
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

namespace ln::shell {

class CLI {
public:
    enum class Mode {
        command,
        interpreter,
    };

    struct Config {
        File istream = File(stdin);
        File ostream = File(stdout);
        static constexpr std::size_t printf_buffer_size = 256;
        static constexpr bool regular_response_is_enabled = true;
        bool colored_output = true;
        bool print_result_tags = false;
        Interpreter *interpreter = nullptr;
        static inline std::array<Cmd::List *, 3> default_cmd_lists = {
            &Cmd::get_base_cmd_list(), &Cmd::get_general_cmd_list(),
            &Cmd::get_global_cmd_list()};
        std::span<Cmd::List *> cmd_lists = default_cmd_lists;
        std::string_view command_mode_prompt_str = "$ ";
        std::string_view interpreter_prompt_str = "> ";
        std::string_view interpreter_multiline_prompt_str = ">> ";
    } config;

    explicit CLI(std::span<char> input_line_buf,
                 std::span<char> history_buf = {})
        : input{input_line_buf}, history{history_buf} {
        this->reset();
    }

    void reset();
    void routine();

    void print(const char &c, std::size_t times_to_repeat = 1);
    int print(const char *str);
    int print(std::string_view sv);
    int printf(const char *fmt, ...);

    /** @return {cmd, args} */
    std::tuple<const Cmd *, std::span<const std::string_view>> find_cmd(
        std::span<const std::string_view> args);

    Err execute_line(std::string_view line);

private:
    Err execute(const Cmd &cmd, std::span<const std::string_view> args,
                const char *output_color_escape_sequence =
                    "\e[32m"); // default in green

    char getc_or_handle_escape_sequences();

    bool delete_char();
    bool move_cursor_begin();
    bool move_cursor_end();
    void add_line_to_history(std::string_view line);
    std::ranges::subrange<ln::RingBufferView<char>::iterator>
    get_previous_history_line();
    bool recall_previous_line_from_history();
    bool recall_next_line_from_history();
    bool step_cursor_left();
    bool step_cursor_right();
    bool step_cursor_left_word();
    bool step_cursor_right_word();

    void print_prompt(bool is_multiline = false);
    void print_prompt_multiline();
    void clear_input();
    bool backspace_char();
    /** @return true if actually inserted */
    bool insert(const char &c);

    LOG_MODULE_CLASS_MEMBER(CLI, LOGGER_LEVEL_NOTSET);

    Input input;
    friend class History;
    History history;
    bool previously_called_from_history = false;
    Err last_err = Err::ok;
    Mode mode = Mode::command;
    bool pasting_mode = false;
};
} // namespace ln::shell
