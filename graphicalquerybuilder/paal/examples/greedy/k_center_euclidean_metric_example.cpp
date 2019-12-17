//=======================================================================
// Copyright (c) 2014 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file k_center_euclidean_metric_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-10-8
 */

//! [K Center Example]
#include "paal/greedy/k_center/k_center.hpp"
#include "paal/data_structures/metric/euclidean_metric.hpp"
#include "paal/utils/irange.hpp"

#include <iostream>
#include <vector>

int main() {
    // sample data
    const int PARTS = 2;
    std::vector<std::pair<int, int>> centers,
        vertices = { { 0, 0 }, { 1, 1 }, { 0, 5 }, { 2, 6 } };
    auto m = paal::data_structures::euclidean_metric<int>();
    // solution
    std::cout << paal::greedy::kCenter(m, PARTS, vertices.begin(),
                                       vertices.end(), back_inserter(centers))
              << std::endl;
}
//! [K Center Example]
