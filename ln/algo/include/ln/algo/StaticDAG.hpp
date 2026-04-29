// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <type_traits>

/**
 * @brief Directed Acyclic Graph (DAG) with only static memory allocation.
 */
namespace ln::algo::StaticDAG {

/**
 * @brief Node base class for static Directed Acyclic Graph (DAG). No dynamic
 * memory allocation. Edges are externally owned.
 *
 * @tparam T The derived node type.
 * @tparam max_edges Maximum number of outgoing edges per node.
 */
template <typename T, std::size_t max_edges> class Node {
public:
    static constexpr std::size_t max_out_edges = max_edges;

    constexpr bool add_edge(T &to) {
        if (this->edge_count >= max_edges) {
            return false;
        }
        this->edges[this->edge_count++] = &to;
        return true;
    }

    constexpr std::size_t out_degree() const { return this->edge_count; }

    constexpr T *edge(std::size_t i) const {
        return i < this->edge_count ? this->edges[i] : nullptr;
    }

private:
    std::array<T *, max_edges> edges{};
    std::size_t edge_count = 0;
};

/**
 * @brief Topological sort of a DAG using Kahn's algorithm.
 *
 * No dynamic memory allocation. Nodes are externally owned.
 * Returns std::nullopt if a cycle is detected.
 *
 * @param nodes  Span of non-owning pointers to all nodes in the graph.
 * @return Topologically sorted node pointers, or std::nullopt if cycle
 * detected.
 */
template <typename T, std::size_t N>
[[nodiscard]] constexpr std::optional<std::array<T *, N>> topo_sort(
    std::span<T *const, N> nodes) {
    static_assert(std::is_base_of_v<Node<T, T::max_out_edges>, T>,
                  "T must derive from ln::DAG::Node<T, max_edges>");

    std::array<std::size_t, N> node_in_degrees{};

    auto index_of = [&](const T *node) -> std::optional<std::size_t> {
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i] == node) {
                return i;
            }
        }
        return std::nullopt;
    };

    // Compute in-degrees
    for (const T *n : nodes) {
        for (std::size_t e = 0; e < n->out_degree(); ++e) {
            const auto opt_j = index_of(n->edge(e));
            if (!opt_j) {
                continue;
            }
            const auto j = *opt_j;
            ++node_in_degrees[j];
        }
    }

    // Kahn's queue (index-based, static)
    std::array<std::size_t, N> queue{};
    std::size_t q_head = 0;
    std::size_t q_tail = 0;

    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (node_in_degrees[i] == 0) {
            queue[q_tail++] = i;
        }
    }

    std::array<T *, N> order{};
    std::size_t order_count = 0;

    while (q_head < q_tail) {
        std::size_t idx = queue[q_head++];
        order[order_count++] = nodes[idx];

        const T *n = nodes[idx];
        for (std::size_t e = 0; e < n->out_degree(); ++e) {
            const auto opt_j = index_of(n->edge(e));
            if (!opt_j) {
                continue;
            }
            const auto j = *opt_j;
            if (--node_in_degrees[j] == 0) {
                queue[q_tail++] = j;
            }
        }
    }

    if (order_count != nodes.size()) {
        return std::nullopt; // cycle detected
    }

    return order;
}

// Overload for non-const span (e.g. std::array<T*,N> lvalue)
template <typename T, std::size_t N>
[[nodiscard]] constexpr std::optional<std::array<T *, N>> topo_sort(
    std::span<T *, N> nodes) {
    return topo_sort(std::span<T *const, N>{nodes});
}

} // namespace ln::algo::StaticDAG
