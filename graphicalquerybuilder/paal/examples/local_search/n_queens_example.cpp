//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 *
 * @file n_queens_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-01-02
 */

#include "paal/local_search/n_queens/n_queens_local_search.hpp"

#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/algorithm/random_shuffle.hpp>

void print(const std::vector<int> &queens) {
    for (int q : queens) {
        for (int i = 0; i < q; ++i) {
            std::cout << "-";
        }
        std::cout << "x";
        for (int i = 0; i < int(queens.size()) - q - 1; ++i) {
            std::cout << "-";
        }
        std::cout << std::endl;
    }
}

int main() {
    namespace ls = paal::local_search;

    // creating initial solution
    std::vector<int> queens(9);
    boost::iota(queens, 0);
    boost::random_shuffle(queens);

    // print before
    std::cout << "start configuration" << std::endl;
    print(queens);

    // local search
    ls::n_queens_local_search_components<> comps;
    ls::n_queens_solution_first_improving(queens, comps);

    // print after
    std::cout << "final configuration" << std::endl;
    print(queens);
}
