// Copyright (c)  2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "ln/shell/CLI.hpp"

#include "FreeRTOS/Addons/Clock.hpp"

void print_run_time_stats(ln::shell::CLI &cli) {
    auto uxCurrentNumberOfTasks = uxTaskGetNumberOfTasks();
    UBaseType_t uxArraySize = uxCurrentNumberOfTasks;
    TaskStatus_t *pxTaskStatusArray = static_cast<TaskStatus_t *>(
        pvPortMalloc(uxCurrentNumberOfTasks * sizeof(TaskStatus_t)));
    if (!pxTaskStatusArray) {
        return;
    }
    uint32_t ulTotalTime = 0UL;
    uxArraySize =
        uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalTime);
    ulTotalTime /= 100UL;
    if (ulTotalTime == 0) {
        return;
    }
    std::string_view line_fmt = "{:<16}{:<12}{:<12}\n";
    cli.print(line_fmt, "Task", "time,.1ms", "time,%");
    for (UBaseType_t x = 0; x < uxArraySize; x++) {
        uint32_t ulStatsAsPercentage =
            pxTaskStatusArray[x].ulRunTimeCounter / ulTotalTime;
        if (ulStatsAsPercentage > 0UL) {
            cli.print(line_fmt, pxTaskStatusArray[x].pcTaskName,
                      pxTaskStatusArray[x].ulRunTimeCounter,
                      ulStatsAsPercentage);
        }
        else {
            cli.print(line_fmt, pxTaskStatusArray[x].pcTaskName,
                      pxTaskStatusArray[x].ulRunTimeCounter, "<1%");
        }
    }
    vPortFree(pxTaskStatusArray);
}

namespace ln::shell {

Cmd status_cmd{Cmd::Cfg{.name = "status,s",
                        .short_description = "show system status",
                        .fn = [](Cmd::Ctx ctx) {
                            ctx.cli.print("uptime: {}\n", ln::get_uptime_ms());
                            ctx.cli.print("freeHeapSize: {}\n\r",
                                          xPortGetFreeHeapSize());
                            ctx.cli.print("RunTimeStatsCntVal: {}\n",
                                          uint32GetRunTimeCounterValue());
                            print_run_time_stats(ctx.cli);
                            return Err::ok;
                        }}};

} // namespace ln::shell
