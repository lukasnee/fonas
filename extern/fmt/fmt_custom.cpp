// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

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
