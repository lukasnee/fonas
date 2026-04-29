// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "ln/ln.h"

extern "C" void ln_panic(const char *file, int line, const char *message) {
    ln::panic(file, line, message);
}
extern "C" void ln_reset() { ln::reset(); }
