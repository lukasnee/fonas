/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/ln.h"
#include "ln/port.h"

#include "FreeRTOS/Addons/Clock.hpp"

extern "C"
{
#include "stm32f3xx.h"
}

#include <cstdio>
#include <cstdint>
#include <inttypes.h>

namespace ln {

static uint32_t get_sp() {
    uint32_t sp = 0;
    __asm volatile("mov %0, sp \n" : "=r"(sp));
    return sp;
}

static uint32_t get_lr() {
    uint32_t lr = 0;
    __asm volatile("mov %0, lr \n" : "=r"(lr));
    return lr;
}

static uint32_t get_ipsr() {
    uint32_t ipsr = 0;
    __asm volatile("mrs %0, ipsr \n" : "=r"(ipsr));
    return ipsr;
}

/**
 * @brief Prints words in hexadecimal format.
 *
 * @param indent The string to print at the beginning of each line.
 * @param address The starting address to read from.
 * @param size The total words to print.
 * @param width The number of words to print per line.
 */
static void print_words_in_hex(const char *indent, uint32_t address,
                               size_t size, size_t width) {
    for (size_t i = 0; i < size; i += width) {
        auto addr_ptr = reinterpret_cast<uint32_t *>(address) + i;
        fputs(indent, stdout);
        for (size_t j = 0; j < width; j++) {
            if (i + j < size) {
                printf("%08" PRIX32 " ", addr_ptr[j]);
            }
            else {
                printf("   ");
            }
        }
        printf("\n");
    }
}

struct ExceptionStackFrameBasic {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t return_address;
    uint32_t retpsr;
};

struct ExceptionStackFrameExtended {
    ExceptionStackFrameBasic basic;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t s12;
    uint32_t s13;
    uint32_t s14;
    uint32_t s15;
    uint32_t fpscr;
    uint32_t vpr;
};

union ExceptionStackFrame {
    ExceptionStackFrameBasic basic;
    ExceptionStackFrameExtended extended;
};

static void hardfault_handler(void *exception_stack_frame) {
    ExceptionStackFrame *frame =
        reinterpret_cast<ExceptionStackFrame *>(exception_stack_frame);
    const auto exc_return = get_lr();
    enum class SPSEL {
        main,
        process
    };
    const SPSEL spsel = exc_return & (1 << 2) ? SPSEL::process : SPSEL::main;
    enum Mode {
        thread,
        handler
    };
    const Mode mode = exc_return & (1 << 3) ? Mode::thread : Mode::handler;
    enum class FType {
        basic,
        extended
    };
    const FType ftype = exc_return & (1 << 4) ? FType::basic : FType::extended;

    printf("\nsystem hardfault!\n");
    printf("uptime: %llu ms\n", get_uptime_ms().count());
    const uint32_t sp =
        reinterpret_cast<uint32_t>(frame) +
        (ftype == FType::basic ? sizeof(ExceptionStackFrameBasic)
                               : sizeof(ExceptionStackFrameExtended));
    printf("SP: %08" PRIX32 " (%s)\n", sp,
           spsel == SPSEL::main ? "MSP" : "PSP");
    printf("SPSEL: %s\n", spsel == SPSEL::main ? "main" : "process");
    printf("mode: %s\n", mode == Mode::thread ? "thread" : "handler");

    const std::array<std::pair<uint32_t, const char *>, 22>
        CFSR_bitmask_to_str = {{{SCB_CFSR_USGFAULTSR_Msk, "USGFAULTSR"},
                                {SCB_CFSR_BUSFAULTSR_Msk, "BUSFAULTSR"},
                                {SCB_CFSR_MEMFAULTSR_Msk, "MEMFAULTSR"},
                                {SCB_CFSR_MMARVALID_Msk, "MMARVALID"},
                                {SCB_CFSR_MLSPERR_Msk, "MLSPERR"},
                                {SCB_CFSR_MSTKERR_Msk, "MSTKERR"},
                                {SCB_CFSR_MUNSTKERR_Msk, "MUNSTKERR"},
                                {SCB_CFSR_DACCVIOL_Msk, "DACCVIOL"},
                                {SCB_CFSR_IACCVIOL_Msk, "IACCVIOL"},
                                {SCB_CFSR_BFARVALID_Msk, "BFARVALID"},
                                {SCB_CFSR_LSPERR_Msk, "LSPERR"},
                                {SCB_CFSR_STKERR_Msk, "STKERR"},
                                {SCB_CFSR_UNSTKERR_Msk, "UNSTKERR"},
                                {SCB_CFSR_IMPRECISERR_Msk, "IMPRECISERR"},
                                {SCB_CFSR_PRECISERR_Msk, "PRECISERR"},
                                {SCB_CFSR_IBUSERR_Msk, "IBUSERR"},
                                {SCB_CFSR_DIVBYZERO_Msk, "DIVBYZERO"},
                                {SCB_CFSR_UNALIGNED_Msk, "UNALIGNED"},
                                {SCB_CFSR_NOCP_Msk, "NOCP"},
                                {SCB_CFSR_INVPC_Msk, "INVPC"},
                                {SCB_CFSR_INVSTATE_Msk, "INVSTATE"},
                                {SCB_CFSR_UNDEFINSTR_Msk, "UNDEFINSTR"}}};

    printf("CFSR: %08" PRIX32 " (", SCB->CFSR);
    bool first = true;
    for (const auto &[bitmask, name] : CFSR_bitmask_to_str) {
        if (SCB->CFSR & bitmask) {
            if (!first) {
                printf("|");
            }
            printf("%s", name);
            first = false;
        }
    }
    printf(")\n");
    printf("HFSR: %08" PRIX32 "\n"
           "DFSR: %08" PRIX32 "\n"
           "MMFAR: %08" PRIX32 "\n"
           "BFAR: %08" PRIX32 "\n"
           "AFSR: %08" PRIX32 "\n",
           SCB->HFSR, SCB->DFSR, SCB->MMFAR, SCB->BFAR, SCB->AFSR);

    printf("exception stack frame:\n"
           "  R0: %08" PRIX32 "\n"
           "  R1: %08" PRIX32 "\n"
           "  R2: %08" PRIX32 "\n"
           "  R3: %08" PRIX32 "\n"
           "  R12: %08" PRIX32 "\n"
           "  LR: %08" PRIX32 "\n"
           "  return_address: %08" PRIX32 "\n"
           "  retpsr: %08" PRIX32 "\n",
           frame->basic.r0, frame->basic.r1, frame->basic.r2, frame->basic.r3,
           frame->basic.r12, frame->basic.lr, frame->basic.return_address,
           frame->basic.retpsr);
    if (ftype == FType::extended) {
        printf("  S0: %08" PRIX32 "\n"
               "  S1: %08" PRIX32 "\n"
               "  S2: %08" PRIX32 "\n"
               "  S3: %08" PRIX32 "\n"
               "  S4: %08" PRIX32 "\n"
               "  S5: %08" PRIX32 "\n"
               "  S6: %08" PRIX32 "\n"
               "  S7: %08" PRIX32 "\n"
               "  S8: %08" PRIX32 "\n"
               "  S9: %08" PRIX32 "\n"
               "  S10: %08" PRIX32 "\n"
               "  S11: %08" PRIX32 "\n"
               "  S12: %08" PRIX32 "\n"
               "  S13: %08" PRIX32 "\n"
               "  S14: %08" PRIX32 "\n"
               "  S15: %08" PRIX32 "\n"
               "  FPSCR: %08" PRIX32 "\n"
               "  VPR: %08" PRIX32 "\n",
               frame->extended.s0, frame->extended.s1, frame->extended.s2,
               frame->extended.s3, frame->extended.s4, frame->extended.s5,
               frame->extended.s6, frame->extended.s7, frame->extended.s8,
               frame->extended.s9, frame->extended.s10, frame->extended.s11,
               frame->extended.s12, frame->extended.s13, frame->extended.s14,
               frame->extended.s15, frame->extended.fpscr, frame->extended.vpr);
    }
    printf("stack dump:\n");
    const size_t words_to_dump = 64;
    print_words_in_hex("  ", sp, words_to_dump, 4);
    reset();
}

[[noreturn]] void panic(const char *file, int line, const char *message) {
    const auto sp = get_sp();
    const auto ipsr = get_ipsr();
    printf("\n%s:%u: panic%s%s\n", file, line, (message ? ": " : "!"),
           (message ? message : ""));
    printf("uptime: %llu ms\n", get_uptime_ms().count());
    printf("SP: %08" PRIX32 "\n", sp);
    printf("IPSR: %u (%s mode)\n", ipsr,
           (ipsr == 0 ? "thread" : "handler/interrupt"));
    printf("stack dump:\n");
    const size_t words_to_dump = 64;
    print_words_in_hex("  ", sp, words_to_dump, 4);
    reset();
    std::unreachable();
}

void reset() {
    __NVIC_SystemReset();
    while (true) {
    }
}

std::chrono::milliseconds get_uptime_ms() {
    return duration_cast<std::chrono::milliseconds>(
        FreeRTOS::Addons::Clock::now().time_since_epoch());
}

} // namespace ln

extern "C" void vApplicationMallocFailedHook(void) { LN_PANIC(); }

extern "C" void _ln_hardfault_handler(void *frame) {
    ln::hardfault_handler(frame);
}
