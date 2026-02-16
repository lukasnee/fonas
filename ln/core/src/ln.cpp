/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/ln.h"

extern "C" void ln_panic(const char *file, int line, const char *message) {
    ln::panic(file, line, message);
}
extern "C" void ln_reset() { ln::reset(); }
