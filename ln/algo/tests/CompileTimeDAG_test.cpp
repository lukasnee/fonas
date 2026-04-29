// Copyright (c) 2026 Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "ln/algo/CompileTimeDAG.hpp"

#include <catch2/catch_test_macros.hpp>

#include <vector>

std::vector<std::string> tasks_executed;

#define TEST_TASK_FN(name)                                                     \
    void name() { tasks_executed.push_back(#name); }

TEST_TASK_FN(ll_uart1)
TEST_TASK_FN(ll_spi1)
TEST_TASK_FN(uart1_driver)
TEST_TASK_FN(spi1_drv)
TEST_TASK_FN(init_hw)
TEST_TASK_FN(load_dr)
TEST_TASK_FN(start_k)
TEST_TASK_FN(serial_app)

TEST_CASE("real-world example", "[ln::algo::CompileTimeDAG]") {

    enum class Step {
        ll_uart1,
        ll_spi1,
        uart1_driver,
        spi1_drv,
        init_hw,
        load_dr,
        start_k,
        serial_app
    };

    struct Task {
        Step key;
        void (*fn)();
    };

    constexpr std::array tasks = {
        Task{Step::ll_uart1, ll_uart1},
        Task{Step::ll_spi1, ll_spi1},
        Task{Step::uart1_driver, uart1_driver},
        Task{Step::spi1_drv, spi1_drv},
        Task{Step::init_hw, init_hw},
        Task{Step::load_dr, load_dr},
        Task{Step::start_k, start_k},
        Task{Step::serial_app, serial_app},
    };

    using Edge = ln::algo::CompileTimeDAG::Edge<Step>;
    constexpr std::array deps = {
        Edge{Step::ll_uart1, Step::uart1_driver},
        Edge{Step::ll_spi1, Step::spi1_drv},
        Edge{Step::uart1_driver, Step::init_hw},
        Edge{Step::spi1_drv, Step::init_hw},
        Edge{Step::init_hw, Step::load_dr},
        Edge{Step::load_dr, Step::start_k},
        Edge{Step::start_k, Step::serial_app},
        Edge{Step::uart1_driver, Step::serial_app},
    };

    constexpr auto ordered_tasks =
        ln::algo::CompileTimeDAG::Graph(tasks, deps).topo_sort();

    static_assert(ordered_tasks[0].key == Step::ll_uart1);
    static_assert(ordered_tasks[1].key == Step::ll_spi1);
    static_assert(ordered_tasks[2].key == Step::uart1_driver);
    static_assert(ordered_tasks[3].key == Step::spi1_drv);
    static_assert(ordered_tasks[4].key == Step::init_hw);
    static_assert(ordered_tasks[5].key == Step::load_dr);
    static_assert(ordered_tasks[6].key == Step::start_k);
    static_assert(ordered_tasks[7].key == Step::serial_app);

    for (const auto &task : ordered_tasks) {
        task.fn();
    }

    REQUIRE(tasks_executed.size() == 8);
    REQUIRE(tasks_executed[0] == "ll_uart1");
    REQUIRE(tasks_executed[1] == "ll_spi1");
    REQUIRE(tasks_executed[2] == "uart1_driver");
    REQUIRE(tasks_executed[3] == "spi1_drv");
    REQUIRE(tasks_executed[4] == "init_hw");
    REQUIRE(tasks_executed[5] == "load_dr");
    REQUIRE(tasks_executed[6] == "start_k");
    REQUIRE(tasks_executed[7] == "serial_app");
}

TEST_CASE("single node, no edges", "[ln::algo::CompileTimeDAG]") {
    enum class N {
        only
    };
    struct Node {
        N key;
        int val;
    };

    constexpr std::array nodes = {Node{N::only, 42}};
    constexpr std::array<ln::algo::CompileTimeDAG::Edge<N>, 0> edges{};

    constexpr auto ordered_nodes =
        ln::algo::CompileTimeDAG::Graph(nodes, edges).topo_sort();

    static_assert(ordered_nodes[0].key == N::only);
    static_assert(ordered_nodes[0].val == 42);
}

TEST_CASE("no edges - order preserved", "[ln::algo::CompileTimeDAG]") {
    // With no edges, all nodes have in-degree 0 and should appear in
    // declaration order (iterating i=0..N-1).
    enum class N {
        a,
        b,
        c
    };
    struct Node {
        N key;
    };

    constexpr std::array nodes = {Node{N::a}, Node{N::b}, Node{N::c}};
    constexpr std::array<ln::algo::CompileTimeDAG::Edge<N>, 0> edges{};

    constexpr auto ordered_nodes =
        ln::algo::CompileTimeDAG::Graph(nodes, edges).topo_sort();

    static_assert(ordered_nodes[0].key == N::a);
    static_assert(ordered_nodes[1].key == N::b);
    static_assert(ordered_nodes[2].key == N::c);
}

TEST_CASE("linear chain", "[ln::algo::CompileTimeDAG]") {
    enum class N {
        a,
        b,
        c,
        d
    };
    struct Node {
        N key;
    };
    using Edge = ln::algo::CompileTimeDAG::Edge<N>;

    constexpr std::array nodes = {Node{N::a}, Node{N::b}, Node{N::c},
                                  Node{N::d}};
    constexpr std::array edges = {
        Edge{N::a, N::b},
        Edge{N::b, N::c},
        Edge{N::c, N::d},
    };

    constexpr auto ordered_nodes =
        ln::algo::CompileTimeDAG::Graph(nodes, edges).topo_sort();

    static_assert(ordered_nodes[0].key == N::a);
    static_assert(ordered_nodes[1].key == N::b);
    static_assert(ordered_nodes[2].key == N::c);
    static_assert(ordered_nodes[3].key == N::d);
}

TEST_CASE("diamond dependency", "[ln::algo::CompileTimeDAG]") {
    // a -> b -> d
    // a -> c -> d
    // 'a' must come first, 'd' must come last, b and c in between
    enum class N {
        a,
        b,
        c,
        d
    };
    struct Node {
        N key;
    };
    using Edge = ln::algo::CompileTimeDAG::Edge<N>;

    constexpr std::array nodes = {Node{N::a}, Node{N::b}, Node{N::c},
                                  Node{N::d}};
    constexpr std::array edges = {
        Edge{N::a, N::b},
        Edge{N::a, N::c},
        Edge{N::b, N::d},
        Edge{N::c, N::d},
    };

    constexpr auto ordered_nodes =
        ln::algo::CompileTimeDAG::Graph(nodes, edges).topo_sort();

    static_assert(ordered_nodes[0].key == N::a);
    // b and c are both valid in positions 1 and 2
    static_assert(ordered_nodes[3].key == N::d);
}

TEST_CASE("multiple roots converge", "[ln::algo::CompileTimeDAG]") {
    // root1 \
    //        -> common -> leaf
    // root2 /
    enum class N {
        root1,
        root2,
        common,
        leaf
    };
    struct Node {
        N key;
    };
    using Edge = ln::algo::CompileTimeDAG::Edge<N>;

    constexpr std::array nodes = {Node{N::root1}, Node{N::root2},
                                  Node{N::common}, Node{N::leaf}};
    constexpr std::array edges = {
        Edge{N::root1, N::common},
        Edge{N::root2, N::common},
        Edge{N::common, N::leaf},
    };

    constexpr auto ordered_nodes =
        ln::algo::CompileTimeDAG::Graph(nodes, edges).topo_sort();

    // roots come before common, common before leaf
    static_assert(ordered_nodes[2].key == N::common);
    static_assert(ordered_nodes[3].key == N::leaf);
}
