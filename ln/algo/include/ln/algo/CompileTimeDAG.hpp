/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <array>
#include <cstddef>
#include <utility>

/**
 * @brief Compile-time Directed Acyclic Graph (DAG).
 */
namespace ln::algo::CompileTimeDAG {

/**
 * @brief Intentionally undefined functions key_to cause compile-time errors in
 * consteval/constexpr contexts.
 */
namespace compile_time_error {
void cycle_detected();
void node_not_found();
} // namespace compile_time_error

template <typename Key> struct Edge {
    Key key_from;
    Key key_to;
};

/**
 * @brief A compile-time DAG implementation using Kahn's algorithm for
 * topological sorting.
 *
 * @tparam Node
 * @tparam Key
 * @tparam node_count
 * @tparam edge_count
 */
template <typename Node, typename Key, size_t node_count, size_t edge_count>
class Graph {

    static_assert(std::is_enum_v<Key>, "Key must be an enum type");
    static_assert(std::is_integral_v<std::underlying_type_t<Key>>,
                  "Underlying enum type must be integral");

public:
    consteval Graph(std::array<Node, node_count> nodes,
                    std::array<Edge<Key>, edge_count> edges)
        : nodes{nodes}, edges{edges} {}

    /**
     * @brief Perform a topological sort using Kahn's algorithm.
     *
     * @retval consteval std::array<Node, N> Sorted nodes in topological order.
     */
    consteval std::array<Node, node_count> topo_sort() const {
        std::array<size_t, node_count> nodes_in_degree{};
        std::array<Node, node_count> sorted{};

        for (const auto &edge : this->edges) {
            nodes_in_degree[find_node_index_by_key(edge.key_to)]++;
        }

        std::array<size_t, node_count> pending{};
        size_t head = 0, tail = 0;

        for (size_t i = 0; i < node_count; ++i) {
            if (nodes_in_degree[i] == 0) {
                pending[tail++] = i;
            }
        }

        size_t count = 0;
        while (head < tail) {
            size_t u = pending[head++];
            sorted[count++] = this->nodes[u];

            for (const auto &edge : this->edges) {
                if (find_node_index_by_key(edge.key_from) == u) {
                    size_t v = find_node_index_by_key(edge.key_to);
                    if (--nodes_in_degree[v] == 0) {
                        pending[tail++] = v;
                    }
                }
            }
        }
        if (count != static_cast<int>(node_count)) {
            compile_time_error::cycle_detected();
            return {};
        }

        return sorted;
    }

private:
    consteval size_t find_node_index_by_key(Key key) const {
        for (int i = 0; i < static_cast<int>(node_count); ++i) {
            if (this->nodes[i].key == key) {
                return i;
            }
        }
        compile_time_error::node_not_found();
        std::unreachable();
        return 0;
    }

    std::array<Node, node_count> nodes;
    std::array<Edge<Key>, edge_count> edges;
};

template <typename Node, typename Key, size_t node_count, size_t edge_count>
Graph(std::array<Node, node_count>, std::array<Edge<Key>, edge_count>)
    -> Graph<Node, Key, node_count, edge_count>;

} // namespace ln::algo::CompileTimeDAG
