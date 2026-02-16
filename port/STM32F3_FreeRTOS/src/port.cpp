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

enum class StackMode {
    main,
    process
};

static StackMode get_stack_mode() {
    return get_lr() & (1 << 2) ? StackMode::process : StackMode::main;
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

typedef struct __attribute__((packed)) {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t return_address;
    uint32_t xpsr;
} ExceptionStackFrame;

static void hardfault_handler(void *exception_stack_frame) {
    ExceptionStackFrame *frame =
        reinterpret_cast<ExceptionStackFrame *>(exception_stack_frame);
    const StackMode stack_mode = get_stack_mode();
    printf("\nsystem hardfault! (uptime: %llu ms)\n", get_uptime_ms().count());
    printf("exception frame:\n"
           "  R0: %08" PRIX32 "\n"
           "  R1: %08" PRIX32 "\n"
           "  R2: %08" PRIX32 "\n"
           "  R3: %08" PRIX32 "\n"
           "  R12: %08" PRIX32 "\n"
           "  LR: %08" PRIX32 "\n"
           "  return_address: %08" PRIX32 "\n"
           "  xpsr: %08" PRIX32 "\n",
           frame->r0, frame->r1, frame->r2, frame->r3, frame->r12, frame->lr,
           frame->return_address, frame->xpsr);

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

    const uint32_t sp =
        reinterpret_cast<uint32_t>(frame) + sizeof(ExceptionStackFrame);
    printf("SP: %08" PRIX32 " (%s)\n", sp,
           stack_mode == StackMode::main ? "MSP" : "PSP");
    printf("stack dump:\n");
    const size_t words_to_dump = 64;
    print_words_in_hex("  ", sp, words_to_dump, 4);
    reset();
}

static void panic(const char *file, int line) {
    const auto sp = get_sp();
    const StackMode stack_mode = get_stack_mode();
    printf("\n%s:%u: system panic! (uptime: %llu ms)\n", file, line,
           get_uptime_ms().count());
    printf("SP: %08" PRIX32 " (%s)\n", sp,
           (stack_mode == StackMode::main ? "MSP" : "PSP"));
    printf("stack dump:\n");
    const size_t words_to_dump = 64;
    print_words_in_hex("  ", sp, words_to_dump, 4);
    reset();
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
