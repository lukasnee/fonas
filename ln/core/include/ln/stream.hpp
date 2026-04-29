// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <span>

namespace ln {

template <typename T> class OutStream {
public:
    virtual ~OutStream() = default;
    virtual void put(std::span<const T> span) = 0;
    void put(const T &value) { this->put(std::span<const T>(&value, 1)); }
};

template <typename T> class InStream {
public:
    virtual ~InStream() = default;
    virtual T get() = 0;
};

template <typename T> class Stream : public OutStream<T>, public InStream<T> {
public:
    virtual ~Stream() = default;
};

} // namespace ln
