//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file bounded_degree_mst_example.cpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2013-11-21
 */


//! [Bounded-Degree Minimum Spanning Tree Example]
#include "paal/iterative_rounding/bounded_degree_min_spanning_tree/bounded_degree_mst.hpp"
#include "paal/utils/functors.hpp"

#include <boost/graph/adjacency_list.hpp>

#include <iostream>
#include <vector>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
        boost::undirectedS, boost::no_property,
        boost::property<boost::edge_weight_t, int>>;
    using Edge = boost::graph_traits<Graph>::edge_descriptor;

    // sample problem
    std::vector<std::pair<int, int>> edges {{0,1},{0,2},{1,2},{1,3},{1,4},
        {1,5},{5,0},{3,4}};
    std::vector<int> costs {1,2,1,2,1,1,1,5};
    std::vector<int> bounds {3,2,2,2,2,2};

    Graph g(edges.begin(), edges.end(), costs.begin(), 6);
    auto degree_bounds = paal::utils::make_array_to_functor(bounds);

    std::vector<Edge> result_tree;

    // optional input validity checking
    auto bdmst = paal::ir::make_bounded_degree_mst(
        g, degree_bounds, std::back_inserter(result_tree));
    auto error = bdmst.check_input_validity();
    if (error) {
        std::cerr << "The input is not valid!" << std::endl;
        std::cerr << *error << std::endl;
        return -1;
    }

    // solve it
    auto result = paal::ir::bounded_degree_mst_iterative_rounding(
        g, degree_bounds, std::back_inserter(result_tree));

    // print result
    if (result.first == paal::lp::OPTIMAL) {
        std::cout << "Edges in the spanning tree" << std::endl;
        for (auto e : result_tree) {
            std::cout << "Edge " << e << std::endl;
        }
        std::cout << "Cost of the solution: " << *(result.second) << std::endl;
    } else {
        std::cout << "The instance is infeasible" << std::endl;
    }
    paal::lp::glp::free_env();
    return 0;
}
    //! [Bounded-Degree Minimum Spanning Tree Example]
