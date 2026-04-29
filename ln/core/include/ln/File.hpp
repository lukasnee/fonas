// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "ln/ln.h"

#include <cstdio>
#include <memory>
#include <span>

namespace ln {

/**
 * @brief Simple RAII wrapper class around standard C FILE*.
 */
class File {
public:
    File() = delete;

    /**
     * @brief Construct for standard files: stdin, stdout, stderr.
     */
    explicit File(FILE *file) : file{file, file_deleter} {
        if (!file) {
            LN_PANIC();
        }
        if (file != stdout && file != stdin && file != stderr) {
            LN_PANIC();
        }
    }

    /**
     * @brief Construct for regular files on filesystem.
     */
    explicit File(const char *path, const char *mode)
        : file{fopen(path, mode), file_deleter} {
        if (!this->file) {
            LN_PANIC();
        }
    }

    /**
     * @brief Construct for memory files.
     */
    explicit File(std::span<char> mem_data, const char *mode)
        : file{fmemopen(mem_data.data(), mem_data.size(), mode), file_deleter} {
        if (!this->file) {
            LN_PANIC();
        }
    }

    FILE *c_file() { return this->file.get(); }

    ~File() = default;

private:
    static void file_deleter(FILE *f) {
        if (f && f != stdin && f != stdout && f != stderr) {
            fclose(f); // NOLINT
        }
    }

    std::shared_ptr<FILE> file;
};

} // namespace ln
