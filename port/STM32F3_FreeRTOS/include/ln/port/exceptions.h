/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define LN_HARDFAULT_HANDLER(_x)                                               \
    __asm volatile("tst lr, #4 \n"                                             \
                   "ite eq \n"                                                 \
                   "mrseq r0, msp \n"                                          \
                   "mrsne r0, psp \n"                                          \
                   "b _ln_hardfault_handler \n")

    void _ln_hardfault_handler(void *exception_stack_frame);

#ifdef __cplusplus
}
#endif
