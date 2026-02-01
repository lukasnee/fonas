/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

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
