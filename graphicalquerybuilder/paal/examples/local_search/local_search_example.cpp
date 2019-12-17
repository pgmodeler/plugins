//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file local_search_example.cpp
 * @brief local search example
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-04
 */

//! [Local Search Components Example]
#include "paal/local_search/local_search.hpp"

#include <vector>
#include <iostream>

namespace ls = paal::local_search;
using namespace paal;

int f(int x) { return -x * x + 12 * x - 27; }

struct get_moves {
    const std::vector<int> neighb;

  public:

    get_moves() : neighb({ 10, -10, 1, -1 }) {}

    const std::vector<int> &operator()(int x) const { return neighb; }
};

struct gain {
    int operator()(int s, int u) { return f(s + u) - f(s); }
};

struct commit {
    bool operator()(int &s, int u) {
        s = s + u;
        return true;
    }
};

typedef ls::search_components<get_moves, gain, commit> search_comps;

//! [Local Search Components Example]

int main() {
    //! [Local Search Example]
    // creating solution
    int solution(0);

    // search
    first_improving(solution, search_comps());

    // print
    std::cout << "Local search solution: " << solution << std::endl;
    return 0;
}
    //! [Local Search Example]
