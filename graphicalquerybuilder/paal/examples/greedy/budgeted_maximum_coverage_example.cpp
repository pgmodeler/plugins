/**
 * @file budgeted_maximum_coverage_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-04-08
 */

//! [Budgeted Maximum Coverage Example]
#include <iostream>
#include <vector>
#include <iterator>

#include <boost/range/irange.hpp>
#include "paal/greedy/set_cover/budgeted_maximum_coverage.hpp"

int main() {
    std::vector<std::vector<int>> set_to_elements = {
        { 1 }, { 2 }, { 0, 1 }, { 2, 3, 4 }
    };
    std::vector<int> set_to_cost = { 1, 1, 5, 5 };
    std::vector<int> element_to_weight = { 3, 5, 1, 1, 1 };
    const int BUDGET = 10;
    auto sets = boost::irange(0, 4);
    std::vector<int> result;
    auto element_index = [](int el){return el;};
    auto covered = paal::greedy::budgeted_maximum_coverage(
        sets,
        [&](int set){return set_to_cost[set];},
        [&](int set){return set_to_elements[set];},
        back_inserter(result),
        element_index,
        BUDGET,
        [&](int el){return element_to_weight[el];});
    std::cout << "Covered: " << covered << std::endl;
}
//! [Budgeted Maximum Coverage Example]
