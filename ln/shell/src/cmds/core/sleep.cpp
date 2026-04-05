/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/shell/CLI.hpp"

namespace ln::shell {

static constexpr std::array<Arg, 1> cmd_sleep_args{
    {Arg{.role = Arg::Role::positional,
         .name = "ms",
         .type = Arg::Type::num,
         .description = "Milliseconds to sleep"}}};

Cmd sleep_cmd{Cmd::Cfg{.cmd_list = Cmd::get_general_cmd_list(),
                       .name = "sleep",
                       .usage = "<ms>",
                       .args = cmd_sleep_args,
                       .short_description = "sleep for a given duration",
                       .fn = [](Cmd::Ctx ctx) {
                           int ms{};
                           auto [ptr, ec] = std::from_chars(
                               ctx.args[0].data(),
                               ctx.args[0].data() + ctx.args[0].size(), ms);
                           if (ec != std::errc()) {
                               return Err::badArg;
                           }
                           ln::sleep(std::chrono::milliseconds(ms));
                           return Err::ok;
                       }}};

} // namespace ln::shell
