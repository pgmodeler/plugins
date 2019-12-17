/**
 * @file set_cover_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-02-17
 */
//! [Set Cover Example]
#include <iostream>
#include <vector>
#include <iterator>

#include <boost/range/irange.hpp>

#include "paal/greedy/set_cover/set_cover.hpp"

int main() {
    std::vector<std::vector<int>> set_to_elements = {
        { 1, 2 },
        { 3, 4, 5, 6 },
        { 7, 8, 9, 10, 11, 12, 13, 0 },
        { 1, 3, 5, 7, 9, 11, 13 },
        { 2, 4, 6, 8, 10, 12, 0 }
    };
    std::vector<int> costs = { 1, 1, 1, 1, 1 };
    auto sets = boost::irange(0, 5);
    std::vector<int> result;
    auto element_index = [](int el){return el;};
    auto cost = paal::greedy::set_cover(sets,
                                        [&](int set){return costs[set];},
                                        [&](int set){return set_to_elements[set];},
                                        back_inserter(result),
                                        element_index);
    std::cout << "Cost: " << cost << std::endl;
}
//! [Set Cover Example]
