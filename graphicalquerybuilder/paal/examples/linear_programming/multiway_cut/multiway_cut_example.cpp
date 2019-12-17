//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file multiway_cut_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-11-13
 */

    //! [Multiway Cut Example]
#include "paal/multiway_cut/multiway_cut.hpp"

#include <boost/graph/adjacency_list.hpp>

int main() {
    // sample data
    std::vector<std::pair<int,int>> edges_p{{0,3},{1,3},
                                           {0,4},{2,4},
                                           {1,5},{2,5},
                                           {3,6},{4,6},
                                           {3,7},{5,7},
                                           {4,8},{5,8},
                                           {6,7},{6,8},{7,8}
    };
    const int vertices_num = 9;
    std::vector<int> cost_edges{100,100,100,100,100,100,10,10,10,10,10,10,1,1,1};

    std::vector<int> terminals = { 0, 1, 2 };
    boost::adjacency_list<
        boost::vecS, boost::vecS, boost::undirectedS,
        boost::property<boost::vertex_index_t, int,
                        boost::property<boost::vertex_color_t, int>>,
                    boost::property<boost::edge_weight_t, int>
                    > graph(edges_p.begin(), edges_p.end(), cost_edges.begin(), vertices_num);

    for (std::size_t i = 1; i <= terminals.size(); ++i) {
        put(boost::vertex_color, graph, terminals[i - 1], i);
    }

    //solve
    std::vector<std::pair<int,int>> vertices_parts;
    auto cost_cut = paal::multiway_cut(graph, back_inserter(vertices_parts));

    //print result
    std::cout << "cost cut: " << cost_cut << std::endl;
    std::cout << "vertices (part)" << std::endl;
    for(auto i: vertices_parts) {
        std::cout << "  " << i.first << "      ( " << i.second << " )" << std::endl;
    }
    paal::lp::glp::free_env();
}
    //! [Multiway Cut Example]
