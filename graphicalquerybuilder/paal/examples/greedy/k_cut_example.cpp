//=======================================================================
// Copyright (c) 2014 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file k_cut_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-08-30
 */

    //! [K Cut Example]
#include "paal/greedy/k_cut/k_cut.hpp"

#include <boost/graph/adjacency_list.hpp>

#include <iostream>

int main() {
    // sample data
    std::vector<std::pair<int,int>> edges_p {{1,2},{1,5},{2,3},{2,5},{2,6},
        {3,4},{3,7},{4,7},{4,0},{5,6},{6,7},{7,0}};
    std::vector<int> costs{2,3,3,2,2,4,2,2,2,3,1,3};
    boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                    boost::no_property,
                    boost::property<boost::edge_index_t, std::size_t>
                    > graph(8);
    for (std::size_t i = 0; i < edges_p.size(); ++i) {
        add_edge(edges_p[i].first, edges_p[i].second, i, graph);
    }
    const int parts = 3;

    auto edge_id = get(boost::edge_index, graph);
    auto weight = make_iterator_property_map(costs.begin(), edge_id);

    // solve
    int cost_cut;
    std::vector<std::pair<int,int>> vertices_parts;
    cost_cut = paal::greedy::k_cut(graph, parts, back_inserter(vertices_parts),
            boost::weight_map(weight));

    // alternative form
    // cost_cut = paal::greedy::k_cut(graph, parts, back_inserter(vertices_parts));
    // this works if the graph has and internal edge weight property map

    // print result
    std::cout << "cost cut:" << cost_cut << std::endl;
    std::vector<int> vertices_to_parts;
    for (auto i: vertices_parts) {
        std::cout << i.first << "(" << i.second << "), ";
    }
    std::cout << std::endl;
}
    //! [K Cut Example]
