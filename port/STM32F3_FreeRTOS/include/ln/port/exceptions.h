// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

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
