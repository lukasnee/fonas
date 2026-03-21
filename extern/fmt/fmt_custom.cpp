/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <ln/ln.h>

#include <fmt/format.h>

#ifndef FMT_FUNC
#define FMT_FUNC
#endif

FMT_BEGIN_NAMESPACE

FMT_FUNC void assert_fail([[maybe_unused]] const char *file,
                          [[maybe_unused]] int line,
                          [[maybe_unused]] const char *message) {
    ln::panic(file, line, message);
}

FMT_END_NAMESPACE
