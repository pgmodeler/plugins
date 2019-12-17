//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file local_search_lambda_example.cpp
 * @brief local search example
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-02-04
 */

    //! [Local Search Example]
#include "paal/local_search/local_search.hpp"

#include <vector>
#include <iostream>

int main() {
    namespace ls = paal::local_search;

    auto f = [](int x) { return -x * x + 12 * x - 27; };
    int solution{ 0 };

    const std::vector<int> neighb{ 10, -10, 1, -1 };

    auto getMoves = [neighb](int) {
        return boost::make_iterator_range(neighb.begin(), neighb.end());
    };

    auto gain = [f](int sol, int move) { return f(sol + move) - f(sol); };

    auto commit = [](int & sol, int move) {
        sol = sol + move;
        return true;
    };

    ls::first_improving(solution,
                        ls::make_search_components(getMoves, gain, commit));

    std::cout << "Local search solution: " << solution << std::endl;
    return 0;
}
    //! [Local Search Example]
