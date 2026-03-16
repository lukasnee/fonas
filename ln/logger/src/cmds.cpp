/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/logger/logger.h"
#include "ln/shell/CLI.hpp"

#include "ln/shell/generic/cmds.hpp"

LOG_MODULE(cmd_log, ln::logger::Level::notset);

namespace ln::shell {

static Cmd cmd_log{
    Cmd::Cfg{.name = "log",
             .usage = generic::cmds::on_off_command_usage,
             .short_description = "enable or disable logging",
             .fn = [](Cmd::Ctx ctx) {
                 auto config = ln::logger::Logger::get_instance().get_config();
                 auto result = generic::cmds::on_off_command_parser(
                     config.enabled_run_time, "logging", ctx);
                 if (result != Err::ok) {
                     return result;
                 }
                 if (!ln::logger::Logger::get_instance().set_config(config)) {
                     ctx.cli.print("failed to set logger config\n");
                     return Err::fail;
                 }
                 return Err::ok;
             }}};

Cmd cmd_log_info{Cmd::Cfg{.parent_cmd = &cmd_log,
                          .name = "info",
                          .usage = "<msg:str>",
                          .short_description = "log info message",
                          .fn = [](Cmd::Ctx ctx) {
                              LOG_INFO("{}", ctx.args[0]);
                              return Err::ok;
                          }}};

Cmd cmd_log_warn{Cmd::Cfg{.parent_cmd = &cmd_log,
                          .name = "warn",
                          .usage = "<msg:str>",
                          .short_description = "log warning message",
                          .fn = [](Cmd::Ctx ctx) {
                              LOG_WARNING("{}", ctx.args[0]);
                              return Err::ok;
                          }}};

Cmd cmd_log_err{Cmd::Cfg{.parent_cmd = &cmd_log,
                         .name = "err",
                         .usage = "<msg:str>",
                         .short_description = "log error message",
                         .fn = [](Cmd::Ctx ctx) {
                             LOG_ERROR("{}", ctx.args[0]);
                             return Err::ok;
                         }}};

// TODO: this is ROM inefficient and weird - improve
#define LOG_CONFIG_BOOL_CMD(cmd_name, config_field, description)               \
    Cmd log_##cmd_name##_cmd {                                                 \
        Cmd::Cfg {                                                             \
            .parent_cmd = &cmd_log, .name = #cmd_name,                         \
            .usage = generic::cmds::on_off_command_usage,                      \
            .fn = [](Cmd::Ctx ctx) {                                           \
                auto config = ln::logger::Logger::get_instance().get_config(); \
                if (ctx.args.size() == 0) {                                    \
                    ctx.cli.print(config.config_field ? "1" : "0");            \
                    ctx.cli.print('\n');                                       \
                    return Err::ok;                                            \
                }                                                              \
                auto result = generic::cmds::on_off_command_parser(            \
                    config.config_field, #description, ctx);                   \
                if (result != Err::ok) {                                       \
                    return result;                                             \
                }                                                              \
                if (!ln::logger::Logger::get_instance().set_config(config)) {  \
                    ctx.cli.print("failed to set logger config\n");            \
                    return Err::fail;                                          \
                }                                                              \
                return Err::ok;                                                \
            }                                                                  \
        }                                                                      \
    }
static LOG_CONFIG_BOOL_CMD(color, color, log coloring);
static LOG_CONFIG_BOOL_CMD(prefix, print_header_enabled, log prefix);
#undef LOG_CONFIG_BOOL_CMD
} // namespace ln::shell
