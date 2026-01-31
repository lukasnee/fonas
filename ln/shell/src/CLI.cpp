/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/shell/CLI.hpp"
#include "ln/shell/Parser.hpp"
// TODO: some kind of escape signal mechanism to inform running cmd to exit.

#include <cstdio>
#include <cstring>
#include <type_traits>

namespace ln::shell {

CLI::CLI(std::span<char> input_line_buf, std::span<char> history_buf)
    : input{input_line_buf}, history{history_buf} {
    this->reset();
}

void CLI::print(const char &c, size_t times_to_repeat) {
    while (times_to_repeat--) {
        if (c == '\n') {
            std::fputc('\r', this->config.ostream.c_file());
        }
        std::fputc(c, this->config.ostream.c_file());
    }
    std::fflush(this->config.ostream.c_file());
}

int CLI::print(std::string_view sv) {
    const auto rc = static_cast<int>(
        std::fwrite(sv.data(), 1, sv.size(), this->config.ostream.c_file()));
    std::fflush(this->config.ostream.c_file());
    return rc;
}

int CLI::print(const char *str) {
    const auto rc = std::fputs(str, this->config.ostream.c_file());
    std::fflush(this->config.ostream.c_file());
    return rc;
}

int CLI::printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::array<char, Config::printf_buffer_size> tx_buf;
    vsnprintf(tx_buf.data(), tx_buf.size(), fmt, args);
    int chars_printed = this->print(tx_buf.data());
    va_end(args);
    return chars_printed;
}

std::tuple<const Cmd *, std::span<const std::string_view>> CLI::find_cmd(
    std::span<const std::string_view> args) {
    if (args.empty()) {
        return {};
    }
    if (args[0].empty()) {
        return {};
    }
    for (const auto &cmd_list_ptr : this->config.cmd_lists) {
        if (!cmd_list_ptr) {
            continue;
        }
        std::size_t arg_offset = 0;
        auto cmd_ptr = Cmd::find_cmd_by_name(*cmd_list_ptr, args[arg_offset]);
        if (!cmd_ptr) {
            continue;
        }
        while (args.size() - arg_offset - 1) {
            const Cmd *child_cmd =
                cmd_ptr->find_child_cmd_by_name(args[arg_offset + 1]);
            if (!child_cmd) {
                break;
            }
            arg_offset++;
            cmd_ptr = child_cmd;
        }
        return {cmd_ptr, args.subspan(arg_offset + 1)};
    }
    return {};
}

// TODO: make first arg the name of the function
Err CLI::execute(const Cmd &cmd, const std::span<const std::string_view> args,
                 const char *output_color_escape_sequence) {
    if (!cmd.cfg.fn) {
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RED);
        }
        this->print("command has no function\n");
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RESET);
        }
        return Err::unexpected;
    }
    ArgParser argp{cmd.cfg.args, args};
    if (!argp.validate_arg_composition(this->config.ostream, args)) {
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_YELLOW);
        }
        this->print("bad arguments\n");
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RESET);
        }
        return Err::badArg;
    }
    this->print(output_color_escape_sequence); // response in green
    const auto err = cmd.cfg.fn(Cmd::Ctx{*this, argp, args});
    if (!Config::regular_response_is_enabled) {
        return err;
    }
    if (err == Err::badArg) {
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_YELLOW);
        }
        this->print("bad arguments\n");
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RESET);
        }
        return err;
    }
    if (err == Err::ok) {
        if (this->config.print_result_tags) {
            if (this->config.colored_output) {
                this->print(ANSI_COLOR_GREEN);
            }
            this->print("\nOK");
            if (this->config.colored_output) {
                this->print(ANSI_COLOR_RESET);
            }
        }
    }
    else if (static_cast<std::int8_t>(err) < 0) {
        if (this->config.print_result_tags) {
            if (this->config.colored_output) {
                this->print(ANSI_COLOR_RED);
            }
            this->print("\nFAIL");
            if (err != Err::fail) {
                this->printf(
                    " (%d)",
                    static_cast<std::underlying_type_t<decltype(err)>>(err));
            }
            if (this->config.colored_output) {
                this->print(ANSI_COLOR_RESET);
            }
        }
    }
    return err;
}

void CLI::reset() {
    // Tell terminal to enable bracketed paste mode
    this->print("\033[?2004h");
}

void CLI::routine() {
    while (true) {
        const char c = this->getc_or_handle_escape_sequences();
        if (c == '\x7F') {
            this->backspace_char();
            continue;
        }
        if (' ' <= c && c <= '~') {
            this->insert(c);
            continue;
        }
        if (this->pasting_mode && c == '\r') {
            this->insert('\n');
            continue;
        }
        if (c == '\r') {
            this->print("\r\n");
            auto line = this->input.get();
            this->add_line_to_history(line);
            if (this->mode == Mode::command) {
                this->last_err = this->execute_line(line);
                if (this->last_err == Err::unknownCmd) {
                    if (this->config.colored_output) {
                        this->print(ANSI_COLOR_RED);
                    }
                    this->print("command not found\n");
                    if (this->config.colored_output) {
                        this->print(ANSI_COLOR_RESET);
                    }
                }
            }
            else if (this->mode == Mode::interpreter) {
                if (!this->config.interpreter) {
                    this->print("No interpreter configured.\n");
                }
                const auto err = this->config.interpreter->interpret_line(line);
                if (err == Interpreter::Err::incomplete) {
                    this->last_err = Err::incomplete;
                    this->input.insert('\n');
                    this->print_prompt_multiline();
                    continue;
                }
                if (err == Interpreter::Err::ok) {
                    this->last_err = Err::ok;
                }
                else if (err == Interpreter::Err::compileError ||
                         err == Interpreter::Err::runtimeError) {
                    this->last_err = Err::fail;
                }
                else {
                    this->last_err = Err::unexpected;
                }
            }
            this->input.clear();
            this->print_prompt();
        }
    }
}

Err CLI::execute_line(std::string_view line) {
    if (line.empty()) {
        return Err::ok;
    }
    std::array<std::string_view, ArgParser::Cfg::args_buf_size_default>
        args_buf;
    auto opt_args = ArgParser::tokenize(line, args_buf);
    if (!opt_args) {
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RED);
        }
        this->print("error parsing arguments\n");
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RESET);
        }
        return Err::badArg;
    }
    const auto args = *opt_args;
    if (args.empty()) {
        return Err::ok;
    }
    const auto [cmd_ptr, cmd_args] = this->find_cmd(args);
    if (!cmd_ptr) {
        return Err::unknownCmd;
    }
    return this->execute(*cmd_ptr, cmd_args);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
char CLI::getc_or_handle_escape_sequences() {
    std::array<char, 10> buf;
    std::size_t buf_size = 0;
    auto getc = [&]() -> char {
        const auto c = std::fgetc(this->config.istream.c_file());
        if (buf_size < buf.size()) {
            buf[buf_size++] = c == '\e' ? 'e' : c;
        }
        return c;
    };
    auto handle_unknown = [&]() {
        LOG_WARNING("Unknown escape sequence: %.*s", static_cast<int>(buf_size),
                    buf.data());
    };
    while (true) {
        char c = getc();
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (c == 0x0C) { // Ctrl + L
            this->clear_screen();
            continue;
        }
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (c == 0x1A) {
            if (this->mode == Mode::command) {
                if (!this->config.interpreter) {
                    this->print("No interpreter configured.\n");
                    continue;
                }
                this->mode = Mode::interpreter;
                this->print("\nSwitched to interpreter mode. "
                            "Press Ctrl+Z to exit.\n");
            }
            else if (this->mode == Mode::interpreter) {
                this->mode = Mode::command;
                this->print("\nSwitched to command mode. "
                            "Press Ctrl+Z to enter interpreter mode.\n");
            }
            this->print_prompt();
            continue;
        }
        if (c != '\e') {
            return c;
        }
        c = getc(); /* if this takes too long, we should trigger "escape key"
                       action TODO: implement timeout. For now, we can trigger
                       an action after two consecutive escape characters. */
        if (c == '\e') { // Escape key (twice)
            this->clear_input();
            continue;
        }
        if (c == '\r') { // Alt + Enter
            this->insert('\n');
            continue;
        }
        if (c == '[') {
            c = getc();
            if (c == 'A') { // Up arrow
                this->recall_previous_line_from_history();
                continue;
            }
            if (c == 'B') { // Down arrow
                this->recall_next_line_from_history();
                continue;
            }
            if (c == 'C') { // Right arrow
                this->step_cursor_right();
                continue;
            }
            if (c == 'D') { // Left arrow
                this->step_cursor_left();
                continue;
            }
            if (c == 'H') { // Home key
                this->move_cursor_begin();
                continue;
            }
            if (c == 'F') { // End key
                this->move_cursor_end();
                continue;
            }
            if (c == '1') {
                c = getc();
                if (c != ';') {
                    handle_unknown();
                    continue;
                }
                c = getc();
                if (c != '5') {
                    handle_unknown();
                    continue;
                }
                c = getc();
                if (c == 'C') { // Ctrl + Right arrow
                    this->step_cursor_right_word();
                    continue;
                }
                if (c == 'D') { // Ctrl + Left arrow
                    this->step_cursor_left_word();
                    continue;
                }
            }
            if (c == '2') {
                c = getc();
                if (c == '0') {
                    c = getc();
                    if (c == '0') {
                        c = getc();
                        if (c == '~') { // Bracketed paste start
                            this->pasting_mode = true;
                            continue;
                        }
                    }
                    else if (c == '1') {
                        c = getc();
                        if (c == '~') { // Bracketed paste end
                            this->pasting_mode = false;
                            continue;
                        }
                    }
                }
            }
            if (c == '3') {
                c = getc();
                if (c == '~') { // Delete key
                    this->delete_char();
                    continue;
                }
            }
        }
        handle_unknown();
    }
}

bool CLI::delete_char() {
    if (!this->input.delete_char()) {
        return false;
    }
    this->print(this->input.get().substr(this->input.get_cursor_pos()));
    this->print(" \b");
    this->print('\b', this->input.get().size() - this->input.get_cursor_pos());
    return true;
}

bool CLI::move_cursor_begin() {
    while (this->step_cursor_left()) {
    }
    return true;
}

bool CLI::move_cursor_end() {
    while (this->step_cursor_right()) {
    }
    return true;
}

void CLI::add_line_to_history(std::string_view line) {
    if (std::ranges::equal(line, this->history.get_current_recall_line())) {
        this->previously_called_from_history = true;
    }
    else {
        this->history.add_line(line);
    }
}

std::ranges::subrange<ln::RingBufferView<char>::iterator> CLI::
    get_previous_history_line() {
    if (this->previously_called_from_history) {
        this->previously_called_from_history = false;
        return this->history.get_current_recall_line();
    }
    return this->history.recall_previous();
}

bool CLI::recall_previous_line_from_history() {
    auto line = this->get_previous_history_line();
    if (line.empty()) {
        return false;
    }
    this->clear_input();
    for (const char &c : line) {
        this->insert(c);
    }
    return true;
}

bool CLI::recall_next_line_from_history() {
    auto line = this->history.recall_next();
    if (line.empty()) {
        return false;
    }
    this->clear_input();
    for (const char &c : line) {
        this->insert(c);
    }
    return true;
}

bool CLI::step_cursor_left() {
    if (!this->input.step_left()) {
        return false;
    }
    this->print('\b');
    return true;
}

bool CLI::step_cursor_right() {
    if (!this->input.step_right()) {
        return false;
    }
    this->print(this->input.get().substr(this->input.get_cursor_pos() - 1, 1));
    return true;
}

bool CLI::step_cursor_left_word() {
    char c = this->input.get()[this->input.get_cursor_pos() - 1];
    bool isspace_to_skip = std::isspace(c);
    bool isalnum_to_skip = std::isalnum(c);
    while (this->input.get_cursor_pos() > 0) {
        this->step_cursor_left();
        c = this->input.get()[this->input.get_cursor_pos() - 1];
        if (isspace_to_skip) {
            if (std::isspace(c)) {
                continue;
            }
            break;
        }
        if (isalnum_to_skip) {
            if (std::isalnum(c)) {
                continue;
            }
            break;
        }
        if (std::isspace(c)) {
            isspace_to_skip = true;
            continue;
        }
        if (std::isalnum(c)) {
            isalnum_to_skip = true;
            continue;
        }
        break;
    }
    return true;
}

bool CLI::step_cursor_right_word() {
    char c = this->input.get()[this->input.get_cursor_pos()];
    bool isspace_to_skip = std::isspace(c);
    bool isalnum_to_skip = std::isalnum(c);
    while (this->input.get_cursor_pos() < this->input.get().size()) {
        this->step_cursor_right();
        const char c = this->input.get()[this->input.get_cursor_pos()];
        if (isspace_to_skip) {
            if (std::isspace(c)) {
                continue;
            }
            break;
        }
        if (isalnum_to_skip) {
            if (std::isalnum(c)) {
                continue;
            }
            break;
        }
        if (std::isspace(c)) {
            isspace_to_skip = true;
            continue;
        }
        if (std::isalnum(c)) {
            isalnum_to_skip = true;
            continue;
        }
        break;
    }
    return true;
}

void CLI::print_prompt(bool is_multiline) {
    if (this->config.colored_output) {
        if (this->last_err == Err::ok || this->last_err == Err::incomplete) {
            this->print(ANSI_COLOR_GREEN);
        }
        else {
            this->print(ANSI_COLOR_RED);
        }
    }
    if (this->mode == Mode::command) {
        this->print(this->config.command_mode_prompt_str);
    }
    else if (this->mode == Mode::interpreter) {
        this->print(is_multiline ? this->config.interpreter_multiline_prompt_str
                                 : this->config.interpreter_prompt_str);
    }
    if (this->config.colored_output) {
        this->print(ANSI_COLOR_YELLOW);
    }
}

void CLI::print_prompt_multiline() { this->print_prompt(true); }

void CLI::clear_screen() {
    this->print("\e[2J\e[H");
    this->print_prompt();
    this->print(this->input.get());
    const size_t chars_to_move_back =
        this->input.get().size() - this->input.get_cursor_pos();
    if (chars_to_move_back > 0) {
        this->printf("\e[%zuD", chars_to_move_back);
    }
}

void CLI::clear_input() {
    this->printf("\e[%zuD \e[%zub\e[%zuD", this->input.get_cursor_pos(),
                 this->input.get().size(), this->input.get().size() + 1);
    this->input.clear();
}

/** @return true if actually backspaced */
bool CLI::backspace_char() {
    if (!this->input.backspace_char()) {
        return false;
    }
    this->print("\b");
    this->print(this->input.get().substr(this->input.get_cursor_pos()));
    this->print(" \b");
    this->print('\b', this->input.get().size() - this->input.get_cursor_pos());
    return true;
}

bool CLI::insert(const char &c) {
    if (!this->input.insert(c)) {
        return false;
    }
    this->print(this->input.get().substr(this->input.get_cursor_pos() - 1));
    this->print('\b', this->input.get().size() - this->input.get_cursor_pos());
    return true;
}

} // namespace ln::shell
