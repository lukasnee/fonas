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

Cmd clear_cmd{Cmd::Cfg{.cmd_list = Cmd::get_general_cmd_list(),
                       .name = "clear",
                       .short_description = "clear screen",
                       .fn = [](Cmd::Ctx ctx) {
                           ctx.cli.clear_screen();
                           return Err::ok;
                       }}};

} // namespace ln::shell
