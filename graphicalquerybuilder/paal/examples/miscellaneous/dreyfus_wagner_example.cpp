//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file dreyfus_wagner_example.cpp
 * @brief
 * @author Maciej Andrejczuk
 * @version 1.0
 * @date 2013-08-01
 */
    //! [Dreyfus Wagner Example]
#include "test/test_utils/sample_graph.hpp"

#include "paal/steiner_tree/dreyfus_wagner.hpp"

#include <boost/range/algorithm/copy.hpp>

#include <iostream>

int main() {
    // prepare metric
    typedef sample_graphs_metrics SGM;
    auto gm = SGM::get_graph_metric_steiner();

    // prepare terminals and Steiner vertices
    std::vector<int> terminals = { SGM::A, SGM::B, SGM::C, SGM::D };
    std::vector<int> nonterminals = { SGM::E };

    // run algorithm
    auto dw =
        paal::make_dreyfus_wagner(gm, terminals, nonterminals);
    dw.solve();

    // print result
    std::cout << "Cost = " << dw.get_cost() << std::endl;
    std::cout << "Steiner points:" << std::endl;
    boost::copy(dw.get_steiner_elements(),
              std::ostream_iterator<int>(std::cout, "\n"));
    std::cout << "Edges:" << std::endl;
    for (auto edge : dw.get_edges()) {
        std::cout << "(" << edge.first << "," << edge.second << ")"
                  << std::endl;
    }

    return 0;
}
    //! [Dreyfus Wagner Example]
