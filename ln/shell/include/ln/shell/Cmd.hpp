// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "ln/shell/Parser.hpp"

#include "ln/StaticForwardList.hpp"

#include <functional>
#include <limits>
#include <cstdint>
#include <span>
#include <string_view>

namespace ln::shell {

enum class Err {
#define X(name) name,
#include "internal/Err.x"
#undef X
};

std::string_view to_string(Err err);

class CLI;

class Cmd : public ln::StaticForwardListNode<Cmd> {
public:
    struct Ctx {
        CLI &cli;
        ArgParser &argp;
        std::span<const std::string_view>
            args; // TODO: deprecated and remove because argp has args
    };

    using Fn = std::function<Err(Ctx)>;

    class List {
    public:
        explicit List(std::string_view name) : name{name} {}
        void print_short_help(CLI &cli, std::size_t max_depth = false) const;

    private:
        friend class Cmd;
        std::string_view name = {};
        ln::StaticForwardList<Cmd> data = {};
    };

    struct Cfg {
        /**
         * @brief Command list to register this command to. It can the
         * global_cmd_list (default), a custom command list.
         * @note Required.
         */
        List &cmd_list = Cmd::get_global_cmd_list();

        /**
         * @brief Parent command. It is base command if nullptr (default).
         * @note Optional.
         */
        Cmd *parent_cmd = nullptr;

        /**
         * @brief Command name tokens separated by commas, e.g. "help,?".
         * @note Required.
         */
        const char *name = nullptr;

        /**
         * @brief Command usage string, e.g. "[all|<command_name> [...]]".
         * Prefer using `args` for automatic usage generation. Currently,
         * `usage` overrides `args` for short help generation.
         * @todo Remove `usage` in favor of automatic usage generation from
         * `args`.
         * @note Optional. May be unspecified if the command has no arguments.
         */
        const char *usage = nullptr;

        /**
         * @brief Command arguments parser configuration.
         * @note Optional.
         */
        const std::span<const Arg> args = {};

        /**
         * @brief Short description in just a few words or up to around 60 to
         * make it fit in one line in the help output. E.g. "show command
         * usage".
         * @note Optional.
         */
        const char *short_description = nullptr;

        /**
         * @brief Long description that may include usage examples and command
         * details. https://docopt.org style is recommended.
         * @note Optional.
         */
        const char *long_description = nullptr;

        /**
         * @brief Command function to execute when the command is called.
         * @note Optional, no command by default. Functionless commands can be
         * used for grouping subcommands.
         */
        Fn fn = nullptr;
    };

    explicit Cmd(Cfg cfg);

    void print_args(CLI &cli) const;
    void print_short_help(CLI &cli, std::size_t max_depth = 1,
                          std::size_t depth = 0) const;
    void print_long_help(CLI &cli, std::size_t max_depth = 1,
                         std::size_t depth = 0) const;

    static List &get_base_cmd_list() {
        static List list{"base"};
        return list;
    }
    static List &get_general_cmd_list() {
        static List list{"general"};
        return list;
    }
    static List &get_global_cmd_list() {
        static List list{"global"};
        return list;
    }

private:
    friend CLI;

    static const Cmd *find_cmd_by_name(List cmd_list, std::string_view name);
    const Cmd *find_child_cmd_by_name(std::string_view name) const;
    std::size_t resolve_cmd_depth() const;

    ln::StaticForwardList<Cmd> children_cmd_list;
    Cfg cfg;
};

} // namespace ln::shell
