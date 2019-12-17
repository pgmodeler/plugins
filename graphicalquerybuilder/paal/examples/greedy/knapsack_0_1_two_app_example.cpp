//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_0_1_two_app_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-20
 */

//! [Knapsack Example]
#include "paal/greedy/knapsack_0_1_two_app.hpp"

#include <vector>
#include <iostream>

int main() {

    const int capacity = 6;
    using Objects = std::vector<std::pair<int, int>>;
    Objects objects{ { 1, 3 }, { 2, 2 }, { 3, 65 }, { 1, 1 }, { 2, 2 },
                     { 4, 3 }, { 1, 1 }, { 10, 23 } };
    auto size = [](std::pair<int, int> object) { return object.first; }
    ;
    auto value = [](std::pair<int, int> object) { return object.second; }
    ;

    Objects result;
    std::cout << "Knapsack 0 / 1" << std::endl;
    auto maxValue = paal::knapsack_0_1_two_app(
        objects, capacity, std::back_inserter(result), value, size);

    std::cout << "Max value " << maxValue.first << ", Total size "
              << maxValue.second << std::endl;
    for (auto o : result) {
        std::cout << "{ size = " << o.first << ", value = " << o.second << "} ";
    }
    std::cout << std::endl;

    return 0;
}
//! [Knapsack Example]
