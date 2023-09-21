//
// Created by Crs_1 on 2023/9/21.
//

#include "../../src/interval-tree/interval_tree.hpp"

int main()
{
    using namespace lib_interval_tree;

    // interval_tree <interval <int>>;
    interval_tree_t <int> tree;

    tree.insert(make_safe_interval<int>(21, 16)); // make_safe_interval swaps low and high if not in right order.
    tree.insert({8, 9});
    tree.insert({25, 30});
    tree.insert({5, 8});
    tree.insert({15, 23});
    tree.insert({17, 19});
    tree.insert({26, 26});
    tree.insert({0, 3});
    tree.insert({6, 10});
    tree.insert({19, 20});

    auto it = tree.end();
    it--;


    for (auto const& i : tree)
    {
        std::cout << "[" << i.low() << ", " << i.high() << "]\n";
    }
}