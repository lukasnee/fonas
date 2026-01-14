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
    const auto rc = static_cast<int>(std::fwrite(sv.data(), 1, sv.size(), this->config.ostream.c_file()));
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

std::tuple<const Cmd *, std::span<const std::string_view>> CLI::find_cmd(std::span<const std::string_view> args) {
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
        auto cmd = Cmd::find_cmd_by_name(*cmd_list_ptr, args[arg_offset]);
        if (!cmd) {
            continue;
        }
        while (args.size() - arg_offset - 1) {
            const Cmd *child_cmd = cmd->find_child_cmd_by_name(args[arg_offset + 1]);
            if (!child_cmd) {
                break;
            }
            arg_offset++;
            cmd = child_cmd;
        }
        return {cmd, args.subspan(arg_offset + 1)};
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
                this->printf(" (%d)", static_cast<std::underlying_type_t<decltype(err)>>(err));
            }
            if (this->config.colored_output) {
                this->print(ANSI_COLOR_RESET);
            }
        }
    }
    return err;
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
        if (c == '\r') {
            this->print("\r\n");
            this->execute_line(this->input.get());
            this->input.clear();
            this->print_prompt();
            continue;
        }
    }
}

bool CLI::execute_line(std::string_view line) {
    if (!this->input.get().empty()) {
        if (std::ranges::equal(this->input.get(), this->history.get_current_recall_line())) {
            this->previously_called_from_history = true;
        }
        else {
            this->history.add_line(this->input.get());
        }
    }
    std::array<std::string_view, ArgParser::Cfg::args_buf_size_default> args_buf;
    auto opt_args = ArgParser::tokenize(line, args_buf);
    if (!opt_args) {
        this->last_err = Err::badArg;
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RED);
        }
        this->print("error parsing arguments\n");
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RESET);
        }
        return false;
    }
    const auto args = *opt_args;
    if (args.empty()) {
        this->last_err = Err::ok;
        return true;
    }
    const auto [cmd, cmd_args] = this->find_cmd(args);
    if (!cmd) {
        this->last_err = Err::unknownCmd;
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RED);
        }
        this->print("command not found\n");
        if (this->config.colored_output) {
            this->print(ANSI_COLOR_RESET);
        }
        return false;
    }
    this->last_err = this->execute(*cmd, cmd_args);
    return true;
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
        LOG_WARNING("Unknown escape sequence: %.*s", static_cast<int>(buf_size), buf.data());
    };
    while (true) {
        char c = getc();
        if (c != '\e') {
            return c;
        }
        c = getc(); // if this takes too long, we should trigger on_escape()
        // TODO: implement timeout. For now, we can trigger on_escape() after
        // two consecutive escape characters.
        if (c == '\e') {
            this->on_escape();
            continue;
        }
        if (c == '[') {
            c = getc();
            if (c == 'A') {
                this->on_arrow_up_key();
                continue;
            }
            if (c == 'B') {
                this->on_arrow_down_key();
                continue;
            }
            if (c == 'C') {
                this->on_arrow_right_key();
                continue;
            }
            if (c == 'D') {
                this->on_arrow_left_key();
                continue;
            }
            if (c == 'H') {
                this->on_home_key();
                continue;
            }
            if (c == 'F') {
                this->on_end_key();
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
                if (c == 'C') {
                    this->on_ctrl_arrow_right_key();
                    continue;
                }
                if (c == 'D') {
                    this->on_ctrl_arrow_left_key();
                    continue;
                }
            }
            if (c == '3') {
                c = getc();
                if (c == '~') {
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

bool CLI::on_escape() {
    this->clear_input();
    return true;
}

bool CLI::on_home_key() {
    while (this->on_arrow_left_key()) {
    }
    return true;
}

bool CLI::on_end_key() {
    while (this->on_arrow_right_key()) {
    }
    return true;
}

bool CLI::on_arrow_up_key() {
    decltype((this->history.get_current_recall_line())) line = {};
    if (this->previously_called_from_history) {
        this->previously_called_from_history = false;
        line = this->history.get_current_recall_line();
    }
    else {
        line = this->history.recall_previous();
    }
    if (line.empty()) {
        return false;
    }
    this->clear_input();
    for (const char &c : line) {
        this->insert(c);
    }
    return true;
}

bool CLI::on_arrow_down_key() {
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

bool CLI::on_arrow_left_key() {
    if (!this->input.step_left()) {
        return false;
    }
    this->print('\b');
    return true;
}

bool CLI::on_arrow_right_key() {
    if (!this->input.step_right()) {
        return false;
    }
    this->print(this->input.get().substr(this->input.get_cursor_pos() - 1, 1));
    return true;
}

bool CLI::on_ctrl_arrow_left_key() {
    char c = this->input.get()[this->input.get_cursor_pos() - 1];
    bool isspace_to_skip = std::isspace(c);
    bool isalnum_to_skip = std::isalnum(c);
    while (this->input.get_cursor_pos() > 0) {
        this->on_arrow_left_key();
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

bool CLI::on_ctrl_arrow_right_key() {
    char c = this->input.get()[this->input.get_cursor_pos()];
    bool isspace_to_skip = std::isspace(c);
    bool isalnum_to_skip = std::isalnum(c);
    while (this->input.get_cursor_pos() < this->input.get().size()) {
        this->on_arrow_right_key();
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

void CLI::print_prompt(void) {
    if (this->config.colored_output) {
        this->print(this->last_err == Err::ok ? ANSI_COLOR_GREEN : ANSI_COLOR_RED);
    }
    this->print("> ");
    if (this->config.colored_output) {
        this->print(ANSI_COLOR_YELLOW);
    }
}

void CLI::clear_input() {
    const size_t max_fmt_size = 8;
    std::array<char, max_fmt_size> buf;
    std::snprintf(buf.data(), buf.size(), "\e[%zuD", this->input.get_cursor_pos());
    this->print(buf.data());
    std::snprintf(buf.data(), buf.size(), " \e[%zub", this->input.get().size());
    this->print(buf.data());
    std::snprintf(buf.data(), buf.size(), "\e[%zuD", this->input.get().size() + 1);
    this->print(buf.data());
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
