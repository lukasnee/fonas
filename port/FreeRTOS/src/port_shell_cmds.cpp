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
        ctx.cli.printf("uptime: %lu s\n",
                       duration_cast<std::chrono::seconds>(
                           FreeRTOS::Addons::Clock::now().time_since_epoch())
                           .count());
        ctx.cli.printf("freeHeapSize: %lu\n\r", xPortGetFreeHeapSize());
        ctx.cli.printf("RunTimeStatsCntVal: %lu\n",
                       uint32GetRunTimeCounterValue());
        ctx.cli.printf("Task\t\ttime,.1ms\ttime,%%\n%s", run_time_stats_buf);
        return Err::ok;
    }}};

} // namespace ln::shell
