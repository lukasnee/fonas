/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/lua/VM.hpp"

#include "ln/ln.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <llimits.h>
}

void l_print(lua_State *L) {
    int n = lua_gettop(L);
    if (n > 0) { /* any result to be printed? */
        luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
        lua_getglobal(L, "print");
        lua_insert(L, 1);
        if (lua_pcall(L, n, 0, 0) != LUA_OK) {
            lua_writestringerror("error calling 'print' (%s)",
                                 lua_tostring(L, -1));
        }
    }
};

bool l_is_incomplete(lua_State *L, int status) {
    if (status == LUA_ERRSYNTAX) {
        size_t len;
        const char *msg = lua_tolstring(L, -1, &len);
        std::string_view error{msg, len};
        if (error.ends_with("<eof>")) {
            return true;
        }
    }
    return false;
};

#if 0
extern "C" {
#include <lstate.h>
}
#include "FreeRTOS.h"
#include "task.h"

void *l_alloc_freertos(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud;    /* Not used in this simple example */
    (void)osize; /* Not used in FreeRTOS heap_4/5 */

    if (nsize == 0) {
        vPortFree(ptr);
        return NULL;
    }
    /* FreeRTOS pvPortMalloc does not have a native 'realloc'.
       We must handle the re-allocation manually. */
    if (!ptr) {
        return pvPortMalloc(nsize);
    }
    // Check if we are shrinking or growing
    void *newptr = pvPortMalloc(nsize);
    if (newptr) {
        // Only copy the minimum of old and new size
        size_t copy_size = (osize < nsize) ? osize : nsize;
        memcpy(newptr, ptr, copy_size);
        vPortFree(ptr);
    }
    return newptr;
}
#endif

namespace ln::lua {

VM::VM() {
#if 0
    this->L = lua_newstate(l_alloc_freertos, nullptr, 0);
#else
    this->L = luaL_newstate();
#endif
    if (!this->L) {
        LN_PANIC();
        return;
    }
    luaL_openlibs(this->L);
}

VM::~VM() {
    if (this->L) {
        lua_close(this->L);
        this->L = nullptr;
    }
}

Interpreter::Err VM::interpret(std::string_view str) {
    // TODO: we could probably optimze by only tokenizing first word
    // and checking if it's a known command. If not, we can skip
    // directly to Lua execution.

    // 1) Try as expression: return <str>
    lua_pushlstring(this->L, "return ", strlen("return "));
    lua_pushlstring(this->L, str.data(), str.size());
    lua_concat(this->L, 2);
    int status = luaL_loadbuffer(this->L, lua_tostring(this->L, -1),
                                 lua_rawlen(this->L, -1), "=shell");
    int nresults = 0;
    if (status != LUA_OK) {
        if (l_is_incomplete(this->L, status)) {
            lua_pop(this->L, 2);
            return Err::incomplete;
        }
        lua_pop(this->L, 2);

        // 2) Try as statement/chunk: <str>
        status = luaL_loadbuffer(this->L, str.data(), str.size(), "=shell");
        if (status != LUA_OK) {
            if (l_is_incomplete(this->L, status)) {
                lua_pop(this->L, 1);
                return Err::incomplete;
            }
            puts(lua_tostring(this->L, -1));
            lua_pop(this->L, 1);
            return Err::compileError;
        }
        nresults = 0;
    }
    else {
        lua_remove(this->L, -2); // remove modified str
        nresults = LUA_MULTRET;
    }
    // 3) Run the chunk
    status = lua_pcall(this->L, 0, nresults, 0);
    if (status != LUA_OK) {
        puts(lua_tostring(this->L, -1));
        lua_pop(this->L, 1);
        return Err::runtimeError;
    }
    l_print(this->L);
    return Err::ok;
}

} // namespace ln::lua
