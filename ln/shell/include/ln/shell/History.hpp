// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "ln/RingBuffer.hpp"
#include "ln/shell/Cmd.hpp"

#include <span>

namespace ln::shell {

class History {

public:
    explicit History(std::span<char> history_buf) : ring_buffer(history_buf) {}

    void add(std::string_view entry);
    std::ranges::subrange<ln::RingBufferView<char>::iterator> get();
    std::ranges::subrange<ln::RingBufferView<char>::iterator> previous();
    std::ranges::subrange<ln::RingBufferView<char>::iterator> next();

private:
    static Err cmd_history_fn(Cmd::Ctx ctx);
    static Cmd cmd_history;

    ln::RingBufferView<char> ring_buffer;
    ln::RingBufferView<char>::iterator recall_it = ring_buffer.end();
    bool last_recall_was_matching = false;
};

} // namespace ln::shell
