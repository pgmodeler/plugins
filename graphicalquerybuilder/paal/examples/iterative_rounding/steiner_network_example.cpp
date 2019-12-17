//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2014 Piotr Godlewski
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_network_example.cpp
 * @brief
 * @author Piotr Wygocki, Piotr Godlewski
 * @version 1.0
 * @date 2013-06-24
 */

//! [Steiner Network Example]
#include "paal/iterative_rounding/steiner_network/steiner_network.hpp"

#include <boost/graph/adjacency_list.hpp>

#include <iostream>
#include <vector>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
        boost::undirectedS, boost::no_property,
        boost::property<boost::edge_weight_t, int>>;
    using Edge = boost::graph_traits<Graph>::edge_descriptor;

    // sample problem
    std::vector<std::pair<int, int>> edges {{0,1},{0,1},{1,2},{1,2},{2,0}};
    std::vector<int> costs {1,1,1,1,7};
    auto restrictions = [](int i, int j) {return 2;};

    Graph g(edges.begin(), edges.end(), costs.begin(), 3);

    std::vector<Edge> result_network;

    // optional input validity checking
    auto steiner_network = paal::ir::make_steiner_network(
        g, restrictions, std::back_inserter(result_network));
    auto error = steiner_network.check_input_validity();
    if (error) {
        std::cerr << "The input is not valid!" << std::endl;
        std::cerr << *error << std::endl;
        return -1;
    }

    // solve it
    auto result = paal::ir::steiner_network_iterative_rounding(
        g, restrictions, std::back_inserter(result_network));

    // print result
    if (result.first == paal::lp::OPTIMAL) {
        std::cout << "Edges in steiner network" << std::endl;
        for (auto e : result_network) {
            std::cout << "Edge " << e << std::endl;
        }
        std::cout << "Cost of the solution: " << *(result.second) << std::endl;
    } else {
        std::cout << "The instance is infeasible" << std::endl;
    }
    paal::lp::glp::free_env();
    return 0;
}
    //! [Steiner Network Example]
