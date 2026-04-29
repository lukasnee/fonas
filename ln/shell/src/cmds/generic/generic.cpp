// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "ln/shell/generic/cmds.hpp"
#include "ln/shell/CLI.hpp"

namespace ln::shell::generic::cmds {

Err on_off_command_parser(std::function<bool(bool)> on_off_fn,
                          const char *ctrl_name, Cmd::Ctx ctx) {
    using namespace std::literals::string_view_literals;
    if (ctx.args.size() != 1) {
        ctx.cli.print("error: no arg\n");
        return Err::badArg;
    }
    if (ctx.args[0] == "on"sv || ctx.args[0] == "1"sv ||
        ctx.args[0] == "true"sv) {
        if (on_off_fn(true)) {
            return Err::ok;
        }
        ctx.cli.print("error: failed to turn on {}\n", ctrl_name);
        return Err::fail;
    }
    if (ctx.args[0] == "off"sv || ctx.args[0] == "0"sv ||
        ctx.args[0] == "false"sv) {
        if (on_off_fn(false)) {
            return Err::ok;
        }
        ctx.cli.print("error: failed to turn off {}\n", ctrl_name);
        return Err::fail;
    }
    ctx.cli.print("error: unexpected arg\n");
    return Err::badArg;
};

Err on_off_command_parser(bool &dst_state, const char *ctrl_name,
                          Cmd::Ctx ctx) {
    return on_off_command_parser(
        [&](bool state) {
            dst_state = state;
            return true;
        },
        ctrl_name, ctx);
};

} // namespace ln::shell::generic::cmds
