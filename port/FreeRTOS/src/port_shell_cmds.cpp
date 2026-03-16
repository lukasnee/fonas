#include "ln/shell/CLI.hpp"

#include "FreeRTOS/Addons/Clock.hpp"

namespace ln::shell {

Cmd status_cmd{Cmd::Cfg{
    .name = "status,s",
    .short_description = "show system status",
    .fn = [](Cmd::Ctx ctx) {
        const size_t buff_size_for_one_task = 20;
        const size_t max_tasks = 40;
        const size_t buff_size = buff_size_for_one_task * max_tasks;
        char run_time_stats_buf[buff_size];
        vTaskGetRunTimeStats(run_time_stats_buf);
        ctx.cli.print("uptime: {}\n", ln::get_uptime_ms());
        ctx.cli.print("freeHeapSize: {}\n\r", xPortGetFreeHeapSize());
        ctx.cli.print("RunTimeStatsCntVal: {}\n",
                      uint32GetRunTimeCounterValue());
        ctx.cli.print("Task\t\ttime,.1ms\ttime,%\n{}", run_time_stats_buf);
        return Err::ok;
    }}};

} // namespace ln::shell
