//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @brief
 * @file knapsack_on_size_fptas_example.cpp
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-20
 */

    //! [Knapsack Example]
#include "paal/dynamic/knapsack_unbounded_fptas.hpp"

#include <vector>
#include <iostream>

int main() {
    using Objects = std::vector<std::pair<int, int>>;
    Objects objects{{1, 3}, {2, 2} , {3, 65} ,
        {1, 1} , {2, 2} , {4, 3} , {1, 1} , {10, 23}};
    const int capacity = 6;
    auto size  = [](std::pair<int, int> object) {return object.first;};
    auto value = [](std::pair<int, int> object) {return object.second;};

    Objects result;
    std::cout << "Knapsack unbounded FPTAS on size" << std::endl;

    double epsilon = 1. / 4.;
    auto maxValue = paal::knapsack_unbounded_on_size_fptas(
        epsilon, objects, capacity,
        std::back_inserter(result), size, value);

    std::cout << "Max value " << maxValue.first << ", Total size "
              << maxValue.second << std::endl;
    for(auto o : result) {
        std::cout << "{ size = " << o.first << ", value = " << o.second << "} ";
    }
    std::cout << std::endl;


    return 0;
}
    //! [Knapsack Example]
