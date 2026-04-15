/*
 * Copyright (c) 2026 Lukas Neverauskis https://github.com/lukasnee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ln/algo/StaticDAG.hpp"

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(bugprone-unchecked-optional-access)

TEST_CASE("add_edge fails when full", "[ln::algo::StaticDAG]") {
    static constexpr std::size_t max_edges = 2;
    struct MyNode : public ln::algo::StaticDAG::Node<MyNode, max_edges> {
        explicit MyNode(int id) : id(id) {}
        int id;
    };
    MyNode a(0), b(1), c(2), d(3);

    REQUIRE(a.add_edge(b));
    REQUIRE(a.add_edge(c));
    REQUIRE_FALSE(a.add_edge(d));
    REQUIRE(a.out_degree() == max_edges);
}

static constexpr std::size_t max_edges = 4;
struct MyNode : public ln::algo::StaticDAG::Node<MyNode, max_edges> {
    explicit MyNode(int id) : id(id) {}
    int id;
};

TEST_CASE("topo_sort single node", "[ln::algo::StaticDAG]") {
    MyNode a(1);
    auto nodes = std::array{&a};

    auto result = ln::algo::StaticDAG::topo_sort(std::span{nodes});
    REQUIRE(result.has_value());
    REQUIRE((*result)[0] == &a);
}

TEST_CASE("topo_sort linear chain", "[ln::algo::StaticDAG]") {
    // a -> b -> c
    MyNode a(1), b(2), c(3);

    a.add_edge(b);
    b.add_edge(c);

    auto nodes = std::array{&a, &b, &c};
    auto result = ln::algo::StaticDAG::topo_sort(std::span{nodes});
    REQUIRE(result.has_value());

    auto &order = *result;
    REQUIRE(order[0] == &a);
    REQUIRE(order[1] == &b);
    REQUIRE(order[2] == &c);
}

TEST_CASE("topo_sort diamond shape", "[ln::algo::StaticDAG]") {
    // a -> b -> d
    // a -> c -> d
    MyNode a(1), b(2), c(3), d(4);

    a.add_edge(b);
    a.add_edge(c);
    b.add_edge(d);
    c.add_edge(d);

    auto nodes = std::array{&a, &b, &c, &d};
    auto result = ln::algo::StaticDAG::topo_sort(std::span{nodes});
    REQUIRE(result.has_value());

    auto &order = *result;
    REQUIRE(order[0] == &a);
    REQUIRE(order[3] == &d);
}

TEST_CASE("topo_sort respects all predecessor constraints",
          "[ln::algo::StaticDAG]") {
    // a -> c, b -> c
    MyNode a(1), b(2), c(3);

    a.add_edge(c);
    b.add_edge(c);

    auto nodes = std::array{&a, &b, &c};
    auto result = ln::algo::StaticDAG::topo_sort(std::span{nodes});
    REQUIRE(result.has_value());

    auto &order = *result;
    std::size_t pos_a = 0, pos_b = 0, pos_c = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (order[i] == &a) {
            pos_a = i;
        }
        if (order[i] == &b) {
            pos_b = i;
        }
        if (order[i] == &c) {
            pos_c = i;
        }
    }
    REQUIRE(pos_a < pos_c);
    REQUIRE(pos_b < pos_c);
}

TEST_CASE("topo_sort detects direct cycle", "[ln::algo::StaticDAG]") {
    // a -> b -> a
    MyNode a(1), b(2);

    a.add_edge(b);
    b.add_edge(a);

    auto nodes = std::array{&a, &b};
    auto result = ln::algo::StaticDAG::topo_sort(std::span{nodes});
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("topo_sort detects longer cycle", "[ln::algo::StaticDAG]") {
    // a -> b -> c -> a
    MyNode a(1), b(2), c(3);

    a.add_edge(b);
    b.add_edge(c);
    c.add_edge(a);

    auto nodes = std::array{&a, &b, &c};
    auto result = ln::algo::StaticDAG::topo_sort(std::span{nodes});
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("topo_sort disconnected nodes", "[ln::algo::StaticDAG]") {
    // a, b, c — no edges
    MyNode a(1), b(2), c(3);

    auto nodes = std::array{&a, &b, &c};
    auto result = ln::algo::StaticDAG::topo_sort(std::span{nodes});
    REQUIRE(result.has_value());
}

TEST_CASE("topo_sort with isolated and connected nodes mixed",
          "[ln::algo::StaticDAG]") {
    // a -> b, c (isolated)
    MyNode a(1), b(2), c(3);

    a.add_edge(b);

    auto nodes = std::array{&a, &b, &c};
    auto result = ln::algo::StaticDAG::topo_sort(std::span{nodes});
    REQUIRE(result.has_value());

    auto &order = *result;
    std::size_t pos_a = 0, pos_b = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (order[i] == &a) {
            pos_a = i;
        }
        if (order[i] == &b) {
            pos_b = i;
        }
    }
    REQUIRE(pos_a < pos_b);
}

// NOLINTEND(bugprone-unchecked-optional-access)
