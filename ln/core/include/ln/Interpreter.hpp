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
        expectingMoreInput
    };

    virtual Err interpret_line(std::string_view line) = 0;
};

} // namespace ln
