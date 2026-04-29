// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "FreeRTOS/Task.hpp"
#include "FreeRTOS/Queue.hpp"

#include <cstdint>

namespace ln::drivers {

class QueueWriter : public FreeRTOS::Task {
public:
    QueueWriter(UBaseType_t size, const char *name,
                configSTACK_DEPTH_TYPE stack_depth, UBaseType_t priority)
        : FreeRTOS::Task(priority, stack_depth, name), queue(size) {}

    bool write_in(const uint8_t &byte) {
        return this->queue.sendToBack(byte, portMAX_DELAY);
    }

protected:
    virtual void write_out(const uint8_t &byte) = 0;

private:
    void taskFunction() final {
        while (true) {
            auto opt_byte = this->queue.receive(portMAX_DELAY);
            if (opt_byte) {
                this->write_out(*opt_byte);
            }
        }
    }

    FreeRTOS::Queue<uint8_t> queue;
};

} // namespace ln::drivers
