//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_tree_ir_example.cpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2014-06-05
 */

//! [IR Steiner Tree Example]
#include "test/test_utils/sample_graph.hpp"

#include "paal/iterative_rounding/steiner_tree/steiner_tree.hpp"

#include <iostream>
#include <vector>

int main() {
    using SGM = sample_graphs_metrics;
    auto metric = SGM::get_graph_metric_steiner();

    std::vector<int> terminals = {SGM::A, SGM::B, SGM::C, SGM::D};
    std::vector<int> nonterminals = {SGM::E};
    std::vector<int> selected_nonterminals;

    // solve it
    paal::ir::steiner_tree_iterative_rounding(metric, terminals,
            nonterminals, std::back_inserter(selected_nonterminals));

    // print result
    std::cout << "Selected vertices:" << std::endl;
    for (auto v : terminals) {
        std::cout << v << std::endl;
    }
    for (auto v : selected_nonterminals) {
        std::cout << v << std::endl;
    }
    auto cost = paal::ir::steiner_utils::count_cost(selected_nonterminals,
            terminals, metric);
    std::cout << "Cost of the solution: " << cost << std::endl;

    paal::lp::glp::free_env();
}

//! [IR Steiner Tree Example]
