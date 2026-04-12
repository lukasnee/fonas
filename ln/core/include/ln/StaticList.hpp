/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <optional>

namespace ln {

template <typename T> class StaticList;

template <typename T> class StaticListNode {
public:
    bool is_linked() const { return this->owner != nullptr; }
    const StaticList<T> *get_owner() const { return this->owner; }

private:
    friend class StaticList<T>;
    StaticListNode<T> *next = nullptr;
    StaticListNode<T> *prev = nullptr;
    StaticList<T> *owner = nullptr;
};

template <typename T> class StaticList {
public:
    template <bool is_const> struct _iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<is_const, const T *, T *>;
        using reference = std::conditional_t<is_const, const T &, T &>;

        using node_pointer =
            std::conditional_t<is_const, const StaticListNode<T> *,
                               StaticListNode<T> *>;

        _iterator() = default;
        explicit _iterator(node_pointer node, node_pointer tail = nullptr)
            : node(node), tail(tail) {}

        template <bool other_const,
                  typename = std::enable_if_t<is_const && !other_const>>
        explicit _iterator(const _iterator<other_const> &other)
            : node(other.node), tail(other.tail) {}

        reference operator*() const { return *static_cast<pointer>(node); }
        pointer operator->() const { return static_cast<pointer>(node); }

        _iterator &operator++() {
            this->tail = this->node;
            this->node = this->node->next;
            return *this;
        }
        _iterator operator++(int) {
            _iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        _iterator &operator--() {
            if (this->node) {
                this->node = this->node->prev;
            }
            else {
                this->node = this->tail; // --end() steps to the last element
            }
            return *this;
        }
        _iterator operator--(int) {
            _iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const _iterator &other) const {
            return node == other.node;
        }
        bool operator!=(const _iterator &other) const {
            return node != other.node;
        }

        node_pointer node = nullptr;
        node_pointer tail = nullptr;
    };

    using iterator = _iterator<false>;
    using const_iterator = _iterator<true>;

    std::optional<iterator> insert(iterator pos, StaticListNode<T> &node) {

        if (node.is_linked()) {
            /* Since this is a static list, inserting a node to multiple lists
            is not possible. */
            return std::nullopt;
        }
        StaticListNode<T> *next = pos != end() ? &(*pos) : nullptr;
        StaticListNode<T> *prev = next ? next->prev : this->tail;
        node.prev = prev;
        node.next = next;
        if (prev) {
            prev->next = &node;
        }
        else {
            this->head = &node;
        }
        if (next) {
            next->prev = &node;
        }
        else {
            this->tail = &node;
        }
        ++this->count;
        node.owner = this;
        return iterator(&node, this->tail);
    }

    std::optional<iterator> push_front(StaticListNode<T> &node) {
        return this->insert(begin(), node);
    }

    std::optional<iterator> push_back(StaticListNode<T> &node) {
        return this->insert(end(), node);
    }

    void remove(StaticListNode<T> &node) {
        if (node.prev) {
            node.prev->next = node.next;
        }
        else {
            this->head = node.next;
        }
        if (node.next) {
            node.next->prev = node.prev;
        }
        else {
            tail = node.prev;
        }
        node.next = nullptr;
        node.prev = nullptr;
        node.owner = nullptr;
        --this->count;
    }

    [[nodiscard]] bool empty() const { return this->count == 0; }
    [[nodiscard]] std::size_t size() const { return this->count; }

    iterator begin() { return iterator(this->head, this->tail); }
    iterator end() { return iterator(nullptr, this->tail); }

    [[nodiscard]] const_iterator begin() const {
        return const_iterator(this->head, this->tail);
    }
    [[nodiscard]] const_iterator end() const {
        return const_iterator(nullptr, this->tail);
    }

    [[nodiscard]] const_iterator cbegin() const {
        return const_iterator(this->head, this->tail);
    }
    [[nodiscard]] const_iterator cend() const {
        return const_iterator(nullptr, this->tail);
    }

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    [[nodiscard]] const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }
    [[nodiscard]] const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    [[nodiscard]] const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }
    [[nodiscard]] const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

private:
    StaticListNode<T> *head = nullptr;
    StaticListNode<T> *tail = nullptr;
    std::size_t count = 0;
};

} // namespace ln
