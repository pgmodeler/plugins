//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file prune_restrictions_to_tree.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-05
 */
#ifndef PAAL_PRUNE_RESTRICTIONS_TO_TREE_HPP
#define PAAL_PRUNE_RESTRICTIONS_TO_TREE_HPP

#include "paal/utils/functors.hpp"
#include "paal/utils/irange.hpp"

#include <boost/function_output_iterator.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

namespace paal {

using RestrictionsVector = std::vector<std::pair<unsigned, unsigned>>;

/**
 * @brief Returns a list of restrictions, made of the edges of a maximum
* spanning tree
 * in a clique with edge weights equal to restriction values between the edges.
 *
 * @tparam Restrictions
 * @param res restrictions
 * @param N number of vertices
 *
 * @return A minimum set of restrictions needed to be checked by the oracle.
 */
template <typename Restrictions>
RestrictionsVector prune_restrictions_to_tree(Restrictions res, int N) {
    using Dist = decltype(std::declval<Restrictions>()(0, 0));
    using EdgeProp = boost::property<boost::edge_weight_t, Dist>;
    using TGraph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                              boost::no_property, EdgeProp>;
    using Edge = typename boost::graph_traits<TGraph>::edge_descriptor;

    RestrictionsVector res_vec;
    TGraph g(N);
    for (auto i : irange(N)) {
        for (auto j : irange(i + 1, N)) {
            add_edge(i, j, EdgeProp(-std::max(res(i, j), res(j, i))), g);
        }
    }

    auto add_edge_to_graph = [&](Edge e) {
        res_vec.push_back(std::make_pair(source(e, g), target(e, g)));
    };

    boost::kruskal_minimum_spanning_tree(
        g, boost::make_function_output_iterator(
               utils::make_assignable_functor(add_edge_to_graph)));
    return res_vec;
}
} //! paal

#endif // PAAL_PRUNE_RESTRICTIONS_TO_TREE_HPP
