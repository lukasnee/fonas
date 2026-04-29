// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

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

    Interpreter::Err interpret(std::string_view str) final;

    lua_State *get_state() { return this->L; }

private:
    lua_State *L = nullptr;
};

} // namespace ln::lua
