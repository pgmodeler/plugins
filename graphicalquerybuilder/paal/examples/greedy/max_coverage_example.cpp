/**
 * @file max_coverage_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-03-13
 */
//! [Max Coverage Example]
#include <iostream>
#include <vector>
#include <iterator>

#include <boost/range/irange.hpp>

#include "paal/greedy/set_cover/maximum_coverage.hpp"

int main() {
    std::vector<std::vector<int>> set_to_elements = {
        { 1, 2 },
        { 3, 4, 5, 6 },
        { 7, 8, 9, 10, 11, 12, 13, 0 },
        { 1, 3, 5, 7, 9, 11, 13 },
        { 2, 4, 6, 8, 10, 12, 0 }
    };
    const int NUMBER_OF_SETS_TO_SELECT = 2;
    auto sets = boost::irange(0, 5);
    using SetIterator = decltype(sets)::iterator;
    std::vector<int> result;
    auto element_index = [](int el){return el;};
    auto covered = paal::greedy::maximum_coverage(
        sets,
        [&](int set){return set_to_elements[set];},
        back_inserter(result),
        element_index,
        NUMBER_OF_SETS_TO_SELECT);
    std::cout << "Covered: " << covered << std::endl;
}
//! [Max Coverage Example]
