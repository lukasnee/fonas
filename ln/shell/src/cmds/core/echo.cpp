/*
 * Copyright (c) 2025 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/shell/CLI.hpp"

#include <fmt/ranges.h>

namespace ln::shell {

Cmd echo_cmd{Cmd::Cfg{.cmd_list = Cmd::get_general_cmd_list(),
                      .name = "echo",
                      .short_description = "echos typed content",
                      .fn = [](Cmd::Ctx ctx) {
                          if (ctx.args.empty()) {
                              ctx.cli.print('\n');
                              return Err::ok;
                          }
                          ctx.cli.print("{}\n", fmt::join(ctx.args, " "));
                          return Err::ok;
                      }}};

} // namespace ln::shell
