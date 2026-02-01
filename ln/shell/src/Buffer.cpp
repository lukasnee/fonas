/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/shell/Buffer.hpp"

#include <cstring>

namespace ln::shell {

void Buffer::clear() {
    this->chars_used = 0;
    this->cursor_idx = 0;
}

std::string_view Buffer::get() const {
    return std::string_view{this->mem.data(), this->chars_used};
}

char Buffer::get_char_at_cursor(ptrdiff_t offset) const {
    if (offset < 0) {
        if (static_cast<ptrdiff_t>(this->cursor_idx) + offset < 0) {
            return '\0';
        }
        return this->mem[this->cursor_idx + offset];
    }
    if (this->cursor_idx + offset >= this->chars_used) {
        return '\0';
    }
    return this->mem[this->cursor_idx + offset];
}

size_t Buffer::get_cursor_pos() const { return this->cursor_idx; }

bool Buffer::is_cursor_on_base() const { return (this->cursor_idx == 0); }

bool Buffer::is_cursor_on_end() const {
    return this->chars_used == this->cursor_idx;
}

bool Buffer::is_empty() const {
    return (this->is_cursor_on_base() && this->is_cursor_on_end());
}

bool Buffer::is_full() const { return (this->chars_used == this->mem.size()); }

bool Buffer::step_right() {
    if (this->is_cursor_on_end()) {
        return false;
    }
    if (this->is_full()) {
        return false;
    }
    this->cursor_idx++;
    return true;
}

bool Buffer::step_left() {
    if (this->is_cursor_on_base()) {
        return false;
    }
    this->cursor_idx--;
    return true;
}

bool Buffer::delete_char() {
    if (this->is_empty() || this->is_cursor_on_end()) {
        return false;
    }
    std::memmove(&this->mem[this->cursor_idx], &this->mem[this->cursor_idx + 1],
                 this->chars_used - (this->cursor_idx + 1));
    this->chars_used--;
    return true;
}

bool Buffer::backspace_char() {
    if (this->is_cursor_on_base()) {
        return false;
    }
    if (!this->is_cursor_on_end()) {
        std::memmove(&this->mem[this->cursor_idx - 1],
                     &this->mem[this->cursor_idx],
                     this->get_distance_to_end() + 1);
    }
    this->cursor_idx--;
    this->chars_used--;
    return true;
}

bool Buffer::insert(char c) {
    if (this->is_full()) {
        return false;
    }
    std::memmove(&this->mem[this->cursor_idx + 1], &this->mem[this->cursor_idx],
                 this->get_distance_to_end());
    this->mem[this->cursor_idx] = c;
    this->cursor_idx++;
    this->chars_used++;
    return true;
}

size_t Buffer::get_distance_to_begin() const { return this->cursor_idx; }

size_t Buffer::get_distance_to_end() const {
    return this->chars_used - this->cursor_idx;
}

size_t Buffer::get_distance_to_prev(char target) const {
    const size_t prev_target_rfind_pos =
        this->get().rfind(target, this->get_cursor_pos() - 1);
    if (prev_target_rfind_pos == std::string_view::npos) {
        return this->get_cursor_pos();
    }
    return this->get_cursor_pos() - prev_target_rfind_pos - 1;
}

size_t Buffer::get_distance_to_next(char target) const {
    const size_t next_target_find_pos =
        this->get().find(target, this->get_cursor_pos());
    if (next_target_find_pos == std::string_view::npos) {
        return this->get().size() - this->get_cursor_pos();
    }
    return next_target_find_pos - this->get_cursor_pos();
}

} // namespace ln::shell
