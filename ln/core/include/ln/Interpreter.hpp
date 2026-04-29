// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string_view>

namespace ln {

class Interpreter {
public:
    enum class Err {
        ok,
        compileError,
        runtimeError,
        incomplete /// String input is incomplete, expects more input.
    };

    /**
     * @brief Interpret a string in REPL (read-eval-print loop) fashion.
     *
     * @param str String to interpret.
     * @return Err Result of interpretation.
     */
    virtual Err interpret(std::string_view str) = 0;
};

} // namespace ln
