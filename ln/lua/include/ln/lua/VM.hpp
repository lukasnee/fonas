/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "ln/Interpreter.hpp"

extern "C"
{
#include <lua.h>
}

namespace ln::lua {

class VM : public Interpreter {
public:
    explicit VM();
    virtual ~VM();

    Interpreter::Err interpret_line(std::string_view line) final;

    lua_State *get_state() { return this->L; }

private:
    lua_State *L = nullptr;
};

} // namespace ln::lua
