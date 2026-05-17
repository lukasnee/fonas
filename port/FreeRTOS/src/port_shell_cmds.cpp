// Copyright (c)  2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "ln/shell/CLI.hpp"

#include "FreeRTOS/Addons/Clock.hpp"

static std::string_view task_state_to_str(eTaskState state) {
    switch (state) {
    case eRunning:
        return "Running";
    case eReady:
        return "Ready";
    case eBlocked:
        return "Blocked";
    case eSuspended:
        return "Suspended";
    case eDeleted:
        return "Deleted";
    case eInvalid:
        return "Invalid";
    default:
        return "Unknown";
    }
};

void print_run_time_stats(ln::shell::CLI &cli) {
    auto num_of_tasks = uxTaskGetNumberOfTasks();
    UBaseType_t array_size = num_of_tasks;
    TaskStatus_t *tasks_status_arr = static_cast<TaskStatus_t *>(
        pvPortMalloc(num_of_tasks * sizeof(TaskStatus_t)));
    if (!tasks_status_arr) {
        return;
    }
    static uint32_t last_total_time = 0UL;
    uint32_t total_time = 0UL;
    array_size =
        uxTaskGetSystemState(tasks_status_arr, array_size, &total_time);
    float ulDeltaTotalTime =
        static_cast<float>(total_time - last_total_time) / 100.0F;

    std::string_view header_fmt = "{:<16} {:<9} {:<4} {:<5} {:<5}\n";
    std::string_view line_fmt = "{:<16} {:<9} {:<4} {:05.2f} {:<5}\n";
    cli.print(header_fmt, "name", "state", "prio", "time%", "hiWtm");
    for (UBaseType_t x = 0; x < array_size; x++) {
        const auto last_run_time_cnt =
            reinterpret_cast<uint32_t>(pvTaskGetThreadLocalStoragePointer(
                tasks_status_arr[x].xHandle,
                configTHREAD_LOCAL_STORAGE_LAST_RUN_TIME_COUNTER_INDEX));
        const auto delta_run_time_cnt = static_cast<float>(
            tasks_status_arr[x].ulRunTimeCounter - last_run_time_cnt);
        const auto delta_run_time_percentage =
            delta_run_time_cnt / ulDeltaTotalTime;
        cli.print(line_fmt, tasks_status_arr[x].pcTaskName,
                  task_state_to_str(tasks_status_arr[x].eCurrentState),
                  tasks_status_arr[x].uxCurrentPriority,
                  delta_run_time_percentage,
                  tasks_status_arr[x].usStackHighWaterMark);
        vTaskSetThreadLocalStoragePointer(
            tasks_status_arr[x].xHandle,
            configTHREAD_LOCAL_STORAGE_LAST_RUN_TIME_COUNTER_INDEX,
            reinterpret_cast<void *>(tasks_status_arr[x].ulRunTimeCounter));
    }
    last_total_time = total_time;
    vPortFree(tasks_status_arr);
}

namespace ln::shell {

Cmd status_cmd{Cmd::Cfg{.name = "status,s",
                        .short_description = "show system status",
                        .fn = [](Cmd::Ctx ctx) {
                            ctx.cli.print("uptime: {}\n", ln::get_uptime_ms());
                            ctx.cli.print("free heap: {}\n\r",
                                          xPortGetFreeHeapSize());
                            print_run_time_stats(ctx.cli);
                            return Err::ok;
                        }}};

} // namespace ln::shell
