/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <cstddef>
#include <string_view>
#include <span>

namespace ln::shell {

class Input {
public:
    explicit Input(std::span<char> line_buf) : line_buf(line_buf) {}
    Input() = delete;
    Input(const Input &) = delete;
    Input &operator=(const Input &) = delete;
    Input(Input &&) = delete;
    Input &operator=(Input &&) = delete;
    virtual ~Input() = default;

    void clear();

    [[nodiscard]] std::string_view::iterator cursor() const {
        return this->line_buf.data() + this->cursor_idx;
    }
    [[nodiscard]] std::string_view get() const;
    [[nodiscard]] char get_char_at_cursor(ptrdiff_t offset = 0) const;
    [[nodiscard]] size_t get_cursor_pos() const; // TODO: maaybe remove?

    [[nodiscard]] bool is_full() const;
    [[nodiscard]] bool is_empty() const;

    [[nodiscard]] bool is_cursor_on_base() const;
    [[nodiscard]] bool is_cursor_on_end() const;

    bool step_right();
    bool step_left();

    bool delete_char();
    bool backspace_char();
    bool insert(char c);

    [[nodiscard]] size_t get_distance_to_begin() const;
    [[nodiscard]] size_t get_distance_to_end() const;
    [[nodiscard]] size_t get_distance_to_prev(char target) const;
    [[nodiscard]] size_t get_distance_to_next(char target) const;

private:
    std::span<char> line_buf;
    std::size_t cursor_idx = 0;
    std::size_t chars_used = 0;
};

} // namespace ln::shell
