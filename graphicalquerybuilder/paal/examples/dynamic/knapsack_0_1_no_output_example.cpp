//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_0_1_no_output_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-20
 */

//! [Knapsack Example]
#include "paal/dynamic/knapsack_0_1.hpp"

#include <vector>
#include <iostream>

int main() {
    using Objects = std::vector<std::pair<int, int>>;
    Objects objects{ { 1, 3 }, { 2, 2 }, { 3, 65 }, { 1, 1 }, { 2, 2 },
                     { 4, 3 }, { 1, 1 }, { 10, 23 } };
    const int capacity = 6;
    auto size = [](std::pair<int, int> object) { return object.first; }
    ;
    auto value = [](std::pair<int, int> object) { return object.second; }
    ;

    std::cout << "Knapsack 0 / 1 no output" << std::endl;
    auto maxValue =
        paal::knapsack_0_1_no_output(objects, capacity, size, value);

    std::cout << "Max value " << maxValue.first << ", Total size "
              << maxValue.second << std::endl;

    return 0;
}
//! [Knapsack Example]
