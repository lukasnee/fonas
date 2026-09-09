// Copyright (c)  2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "ln/prof/prof.h"

#ifndef LN_PROF_ITM_PORT
#define LN_PROF_ITM_PORT 0
#endif
#include "ln/ln.h"

#include "ln/port/itm.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

struct Packet {
    enum class Type : uint32_t {
        enter = 0,
        exit = 1,
    };

    static constexpr std::uint8_t SYNC_BYTE = 0xA5;

#pragma pack(push, 1)
    union {
        struct {
            uint32_t sync_byte : 8;
            uint32_t cycle_cnt_delta : 24;
            uint32_t type : 1;
            uint32_t context : 7;
            uint32_t fn : 24; // MSB shall be provided by the other packet
        };
        uint32_t as_u32[2];
    };
#pragma pack(pop)
};

// Attention: performance is of utmost importance

volatile bool started = false;

void ln_prof_start() {

    // Enable trace (required for ITM and DWT)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // Reset and enable cycle counter (used by ln/prof for timestamps)
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    // Unlock ITM
    ITM->LAR = 0xC5ACCE55;
    // Enable ITM
    ITM->TCR |= ITM_TCR_ITMENA_Msk;
    // Enable stimulus port 0 (LN_PROF_ITM_PORT)
    ITM->TER |= (1UL << LN_PROF_ITM_PORT);

    if (!LN_ITM_IS_PORT_READY(LN_PROF_ITM_PORT)) {
        ITM->TER &= ~(1UL << LN_PROF_ITM_PORT); // undo
        return;
    }
    started = true;
}

void ln_prof_stop() {
    ITM->TER &= ~(1UL << LN_PROF_ITM_PORT);

    started = false;
}

static uint32_t last_cycle_count = 0;

extern "C" LN_PROF_ATTR void __cyg_profile_func_enter(void *this_fn,
                                                      void *call_site) {
    (void)call_site;
    if (!started) {
        return;
    }
    if (ln::interrupt_context()) {
        return;
    }
    uint32_t old_primask = __get_PRIMASK();
    ln::disable_irq();
    const uint32_t cycle_count = DWT->CYCCNT;
    Packet packet{
        .sync_byte = Packet::SYNC_BYTE,
        .cycle_cnt_delta = cycle_count - last_cycle_count,
        .type = static_cast<uint32_t>(Packet::Type::enter),
        .context = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle()),
        .fn = reinterpret_cast<uint32_t>(this_fn) & 0x00FFFFFF,
    };
    last_cycle_count = cycle_count;
    LN_ITM_SEND_WORD(LN_PROF_ITM_PORT, packet.as_u32[0]);
    LN_ITM_SEND_WORD(LN_PROF_ITM_PORT, packet.as_u32[1]);
    __set_PRIMASK(old_primask);
}

extern "C" LN_PROF_ATTR void __cyg_profile_func_exit(
    void *this_fn, [[maybe_unused]] void *call_site) {
    if (!started) {
        return;
    }
    if (ln::interrupt_context()) {
        return;
    }
    uint32_t old_primask = __get_PRIMASK();
    ln::disable_irq();
    const uint32_t cycle_count = DWT->CYCCNT;
    Packet packet{
        .sync_byte = Packet::SYNC_BYTE,
        .cycle_cnt_delta = cycle_count - last_cycle_count,
        .type = static_cast<uint32_t>(Packet::Type::exit),
        .context = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle()),
        .fn = reinterpret_cast<uint32_t>(this_fn) & 0x00FFFFFF,
    };
    last_cycle_count = cycle_count;
    LN_ITM_SEND_WORD(LN_PROF_ITM_PORT, packet.as_u32[0]);
    LN_ITM_SEND_WORD(LN_PROF_ITM_PORT, packet.as_u32[1]);
    __set_PRIMASK(old_primask);
}
