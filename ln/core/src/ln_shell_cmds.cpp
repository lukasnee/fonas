/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/shell/CLI.hpp"
#include "ln/ln.h"

namespace ln::shell {

Cmd reset_cmd{Cmd::Cfg{.name = "reset",
                       .short_description = "soft system reset",
                       .fn = []([[maybe_unused]]
                                Cmd::Ctx ctx) {
                           ln::reset();
                           return Err::unexpected;
                       }}};

Cmd panic_cmd{Cmd::Cfg{.name = "test_fault",
                       .short_description = "test fault behavior (handling)",
                       .long_description = R"(Usage:
  test_fault panic
  test_fault read_invalid_address

Commands:
  panic                  test panic behavior
  read_invalid_address   test hardfault behavior by reading from an invalid address
)",
                       .fn = [](Cmd::Ctx ctx) {
                           if (ctx.args.size() != 1) {
                               return Err::badArg;
                           }
                           using namespace std::literals::string_view_literals;
                           if (ctx.args[0] == "panic"sv) {
                               LN_PANIC_WITH_MSG("this is a test");
                           }
                           else if (ctx.args.size() == 1 &&
                                    ctx.args[0] == "read_invalid_address"sv) {
                               const auto invalid_address = 0xbadcafe;
                               return *reinterpret_cast<Err *>(invalid_address);
                           }
                           ctx.cli.print("unknown subcommand '{}'\n",
                                         ctx.args[0]);
                           return Err::badArg;
                       }}};

Cmd uptime_cmd{Cmd::Cfg{.name = "uptime",
                        .short_description = "show system uptime",
                        .fn = [](Cmd::Ctx ctx) {
                            ctx.cli.print("{}\n", ln::get_uptime_ms());
                            return Err::ok;
                        }}};

} // namespace ln::shell
