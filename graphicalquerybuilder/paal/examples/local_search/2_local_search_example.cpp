//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file 2_local_search_example.cpp
 * @brief 2-opt example.
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

    //! [Two Local Search Example]
#include "test/test_utils/sample_graph.hpp"

#include "paal/local_search/2_local_search/2_local_search.hpp"
#include "paal/data_structures/cycle/cycle_algo.hpp"
#include "paal/data_structures/cycle/simple_cycle.hpp"

#include <vector>
#include <iostream>

using namespace paal::local_search;
using namespace paal;

int main() {
    // sample data
    typedef sample_graphs_metrics SGM;
    auto gm = SGM::get_graph_metric_small();
    const int size = gm.size();
    std::cout << size << std::endl;
    std::vector<int> v(size);
    std::iota(v.begin(), v.end(), 0);

    // create random solution
    std::random_shuffle(v.begin(), v.end());
    typedef data_structures::simple_cycle<int> Cycle;
    Cycle cycle(v.begin(), v.end());
    std::cout << "Length \t" << get_cycle_length(gm, cycle) << std::endl;

    // search
    tsp_first_improving(cycle, get_default_two_local_components(gm));

    // printing
    std::cout << "Length \t" << get_cycle_length(gm, cycle) << std::endl;

    return 0;
}
    //! [Two Local Search Example]
