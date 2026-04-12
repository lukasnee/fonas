#include "ln/StaticList.hpp"

#include <catch2/catch_test_macros.hpp>

struct Item : public ln::StaticListNode<Item> {
    explicit Item(int v) : value(v) {}
    int value;
};

TEST_CASE("ln::StaticList is empty on construction", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    REQUIRE(list.empty());
    REQUIRE(list.begin() == list.end());
}

TEST_CASE("ln::StaticList push_back single element", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1);

    list.push_back(a);

    REQUIRE_FALSE(list.empty());
    REQUIRE(list.begin()->value == 1);
}

TEST_CASE("ln::StaticList push_front single element", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1);

    list.push_front(a);

    REQUIRE(list.size() == 1);
    REQUIRE_FALSE(list.empty());
    REQUIRE(list.begin()->value == 1);
}

TEST_CASE("ln::StaticList push_back preserves order", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1), b(2), c(3);

    list.push_back(a);
    list.push_back(b);
    list.push_back(c);

    REQUIRE(list.size() == 3);

    auto it = list.begin();
    REQUIRE(it->value == 1);
    ++it;
    REQUIRE(it->value == 2);
    ++it;
    REQUIRE(it->value == 3);
    ++it;
    REQUIRE(it == list.end());
}

TEST_CASE("ln::StaticList push_front reverses insertion order",
          "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1), b(2), c(3);

    list.push_front(a);
    list.push_front(b);
    list.push_front(c);

    REQUIRE_FALSE(list.empty());
    REQUIRE(list.size() == 3);
    auto it = list.begin();
    REQUIRE(it->value == 3);
    ++it;
    REQUIRE(it->value == 2);
    ++it;
    REQUIRE(it->value == 1);
    ++it;
    REQUIRE(it == list.end());
}

TEST_CASE("ln::StaticList remove head", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1), b(2), c(3);

    list.push_back(a);
    list.push_back(b);
    list.push_back(c);

    REQUIRE(list.size() == 3);
    list.remove(a);
    REQUIRE(list.size() == 2);

    auto it = list.begin();
    REQUIRE(it->value == 2);
    ++it;
    REQUIRE(it->value == 3);
    ++it;
    REQUIRE(it == list.end());
}

TEST_CASE("ln::StaticList remove tail", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1), b(2), c(3);

    list.push_back(a);
    list.push_back(b);
    list.push_back(c);

    list.remove(c);

    auto it = list.begin();
    REQUIRE(it->value == 1);
    ++it;
    REQUIRE(it->value == 2);
    ++it;
    REQUIRE(it == list.end());
}

TEST_CASE("ln::StaticList remove middle element", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1), b(2), c(3);

    list.push_back(a);
    list.push_back(b);
    list.push_back(c);

    list.remove(b);

    auto it = list.begin();
    REQUIRE(it->value == 1);
    ++it;
    REQUIRE(it->value == 3);
    ++it;
    REQUIRE(it == list.end());
}

TEST_CASE("ln::StaticList remove only element results in empty list",
          "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1);

    list.push_back(a);
    list.remove(a);

    REQUIRE(list.empty());
    REQUIRE(list.begin() == list.end());
}

TEST_CASE("ln::StaticList reverse iteration via rbegin/rend",
          "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1), b(2), c(3);

    list.push_back(a);
    list.push_back(b);
    list.push_back(c);

    auto it = list.rbegin();
    REQUIRE(it->value == 3);
    ++it;
    REQUIRE(it->value == 2);
    ++it;
    REQUIRE(it->value == 1);
    ++it;
    REQUIRE(it == list.rend());
}

TEST_CASE("ln::StaticList const iteration", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(10), b(20);

    list.push_back(a);
    list.push_back(b);

    const auto &clist = list;
    auto it = clist.begin();
    REQUIRE(it->value == 10);
    ++it;
    REQUIRE(it->value == 20);
    ++it;
    REQUIRE(it == clist.end());
}

TEST_CASE("ln::StaticList re-push after remove", "[ln::StaticList]") {
    ln::StaticList<Item> list;
    Item a(1), b(2);

    list.push_back(a);
    list.push_back(b);
    list.remove(a);
    list.push_back(a);

    auto it = list.begin();
    REQUIRE(it->value == 2);
    ++it;
    REQUIRE(it->value == 1);
    ++it;
    REQUIRE(it == list.end());
}

#include <ranges>

TEST_CASE("ln::StaticList works with std::ranges", "[ln::StaticList]") {
    ln::StaticList<Item> list;

    std::array<Item, 5> items{Item(1), Item(2), Item(3), Item(4), Item(5)};
    for (auto &item : items) {
        list.push_back(item);
    }

    for (auto [idx, item] : list | std::views::enumerate) {
        REQUIRE(item.value == static_cast<int>(idx) + 1);
    }

    for (auto [idx, item] :
         list | std::views::reverse | std::views::enumerate) {
        REQUIRE(item.value == static_cast<int>(list.size() - idx));
    }

    for (auto [idx, item] : list | std::views::filter([](const Item &item) {
                                return item.value % 2 == 1;
                            }) | std::views::enumerate) {
        REQUIRE(item.value % 2 == 1);
        REQUIRE(item.value == static_cast<int>(idx) * 2 + 1);
    }
}
