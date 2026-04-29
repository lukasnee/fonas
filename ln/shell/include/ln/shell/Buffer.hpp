// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <cstddef>
#include <string_view>
#include <span>

namespace ln::shell {

class Buffer {
public:
    explicit Buffer(std::span<char> mem) : mem(mem) {}
    Buffer() = delete;
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(Buffer &&) = delete;
    virtual ~Buffer() = default;

    void clear();

    [[nodiscard]] std::string_view::iterator cursor() const {
        return this->mem.data() + this->cursor_idx;
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
    std::span<char> mem;
    std::size_t cursor_idx = 0;
    std::size_t chars_used = 0;
};

} // namespace ln::shell
