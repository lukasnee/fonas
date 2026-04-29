// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "ln/shell/Cmd.hpp"

namespace ln::shell::generic::cmds {
// TODO: make better API for on_off_command "overriding". Now its a bit clunky.

static constexpr const char *on_off_command_usage = "<on|off>";

Err on_off_command_parser(std::function<bool(bool)> on_off_fn,
                          const char *ctrl_name, Cmd::Ctx ctx);
Err on_off_command_parser(bool &dst_state, const char *ctrl_name, Cmd::Ctx ctx);

} // namespace ln::shell::generic::cmds
