//=======================================================================
// Copyright (c) 2014 Piotr Smulewicz
//               2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_tree_greedy.hpp
 * @brief
 * @author Piotr Wygocki, Piotr Smulewicz
 * @version 1.0
 * @date 2013-11-27
 */
#ifndef PAAL_STEINER_TREE_GREEDY_HPP
#define PAAL_STEINER_TREE_GREEDY_HPP

#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/functors.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/two_bit_color_map.hpp>

#include <boost/property_map/property_map.hpp>

#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/fill.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/as_array.hpp>
#include <boost/range/numeric.hpp>

#include <algorithm>
#include <utility>

/// enum for edge base property
enum edge_base_t { edge_base };

namespace boost {
/// macro create edge base property
BOOST_INSTALL_PROPERTY(edge, base);
}

namespace paal {

/**
 * @brief enum indicates if given color represents terminal or NONTERMINAL.
 */
enum Terminals { NONTERMINAL, TERMINAL };

namespace detail {
template <typename NearestMap, typename LastEdgeMap, typename Tag>
class nearest_recorder
    : boost::base_visitor<nearest_recorder<NearestMap, LastEdgeMap, Tag>> {
  public:
    using event_filter = Tag;
    nearest_recorder(NearestMap &nearest_map, LastEdgeMap &vpred)
        : m_nearest_map(nearest_map), m_vpred(vpred) {};
    template <typename Edge, typename Graph>
    void operator()(Edge const e, Graph const &g) {
        m_nearest_map[target(e, g)] = m_nearest_map[source(e, g)];
        m_vpred[target(e, g)] = e;
    }

  private:
    NearestMap &m_nearest_map;
    LastEdgeMap &m_vpred;
};

template <typename NearestMap, typename LastEdgeMap, typename Tag>
nearest_recorder<NearestMap, LastEdgeMap, Tag>
make_nearest_recorder(NearestMap &nearest_map, LastEdgeMap &vpred, Tag) {
    return nearest_recorder<NearestMap, LastEdgeMap, Tag>{ nearest_map, vpred };
}
}
/**
 * @brief non-named version of  steiner_tree_greedy
 *
 * @tparam Graph
 * @tparam OutputIterator
 * @tparam EdgeWeightMap
 * @tparam ColorMap
 * @param g - given graph
 * @param out - edge output iterator
 * @param edge_weight
 * @param color_map
 */
template <typename Graph, typename OutputIterator, typename EdgeWeightMap,
          typename ColorMap>
auto steiner_tree_greedy(const Graph &g, OutputIterator out,
                         EdgeWeightMap edge_weight, ColorMap color_map)
    -> typename std::pair<
          typename boost::property_traits<EdgeWeightMap>::value_type,
          typename boost::property_traits<EdgeWeightMap>::value_type> {
    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
    using Base = typename boost::property<edge_base_t, Edge>;
    using Weight = typename boost::property_traits<EdgeWeightMap>::value_type;
    using WeightProperty =
        typename boost::property<boost::edge_weight_t, Weight, Base>;
    using TerminalGraph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                              boost::no_property, WeightProperty>;
    using EdgeTerminal =
        typename boost::graph_traits<TerminalGraph>::edge_descriptor;

    auto N = num_vertices(g);

    // distance array used in the dijkstra runs
    std::vector<Weight> distance(N);

    // computing terminals
    std::vector<int> terminals;
    auto terminals_nr = accumulate_functor(
        vertices(g), 0, [=](Vertex v) { return get(color_map, v); });
    terminals.reserve(terminals_nr);
    for (auto v : boost::as_array(vertices(g))) {
        if (get(color_map, v) == Terminals::TERMINAL) {
            terminals.push_back(v);
        }
    }
    if (terminals.empty()) {
        return std::make_pair(Weight{}, Weight{});
    }
    std::vector<Vertex> nearest_terminal(num_vertices(g));
    auto index = get(boost::vertex_index, g);
    auto nearest_terminal_map = boost::make_iterator_property_map(
        nearest_terminal.begin(), get(boost::vertex_index, g));
    for (auto terminal : terminals) {
        nearest_terminal_map[terminal] = terminal;
    }

    // compute voronoi diagram each vertex get nearest terminal and last edge on
    // path to nearest terminal
    auto distance_map = make_iterator_property_map(distance.begin(), index);
    std::vector<Edge> vpred(N);
    auto last_edge = boost::make_iterator_property_map(
        vpred.begin(), get(boost::vertex_index, g));
    boost::dijkstra_shortest_paths(
        g, terminals.begin(), terminals.end(), boost::dummy_property_map(),
        distance_map, edge_weight, index, utils::less(),
        boost::closed_plus<Weight>(), std::numeric_limits<Weight>::max(), 0,
        boost::make_dijkstra_visitor(detail::make_nearest_recorder(
            nearest_terminal_map, last_edge, boost::on_edge_relaxed{})));

    // computing distances between terminals
    // creating terminal_graph
    TerminalGraph terminal_graph(N);
    for (auto w : boost::as_array(edges(g))) {
        auto const &nearest_to_source = nearest_terminal_map[source(w, g)];
        auto const &nearest_to_target = nearest_terminal_map[target(w, g)];
        if (nearest_to_source != nearest_to_target) {
            add_edge(nearest_to_source, nearest_to_target,
                     WeightProperty(distance[source(w, g)] +
                                        distance[target(w, g)] + edge_weight[w],
                                    Base(w)),
                     terminal_graph);
        }
    }
    // computing spanning tree on terminal_graph
    std::vector<Edge> terminal_edge;
    boost::kruskal_minimum_spanning_tree(terminal_graph,
                                         std::back_inserter(terminal_edge));

    // computing result
    std::vector<Edge> tree_edges;
    tree_edges.reserve(terminals_nr);
    for (auto edge : terminal_edge) {
        auto base = get(edge_base, terminal_graph, edge);
        tree_edges.push_back(base);
        for (auto pom : { source(base, g), target(base, g) }) {
            while (nearest_terminal_map[pom] != pom) {
                tree_edges.push_back(vpred[pom]);
                pom = source(vpred[pom], g);
            }
        }
    }

    // because in each voronoi region we have unique patch to all vertex from
    // terminal, result graph contain no cycle
    // and all leaf are terminal

    boost::sort(tree_edges);
    auto get_weight=[&](Edge edge){return edge_weight[edge];};
    auto lower_bound=accumulate_functor(tree_edges, Weight{}, get_weight);
    auto unique_edges = boost::unique(tree_edges);
    auto cost_solution=accumulate_functor(unique_edges, Weight{}, get_weight);
    boost::copy(unique_edges, out);
    return std::make_pair(cost_solution, lower_bound / 2.);
}

/**
 * @brief named version of  steiner_tree_greedy
 *
 * @tparam Graph
 * @tparam OutputIterator
 * @tparam P
 * @tparam T
 * @tparam R
 * @param g - given graph
 * @param out - edge output iterator
 * @param params
 */
template <typename Graph, typename OutputIterator, typename P, typename T,
          typename R>
auto steiner_tree_greedy(const Graph &g, OutputIterator out,
                         const boost::bgl_named_params<P, T, R> &params) {
    return steiner_tree_greedy(
        g, out, choose_const_pmap(get_param(params, boost::edge_weight), g,
                                  boost::edge_weight),
        choose_const_pmap(get_param(params, boost::vertex_color), g,
                          boost::vertex_color));
}

/**
 * @brief version of  steiner_tree_greedy with all default parameters
 *
 * @tparam Graph
 * @tparam OutputIterator
 * @param g - given graph
 * @param out - edge output iterator
 */
template <typename Graph, typename OutputIterator>
auto steiner_tree_greedy(const Graph &g, OutputIterator out) {
    return steiner_tree_greedy(g, out, boost::no_named_parameters());
}

} // paal

#endif // PAAL_STEINER_TREE_GREEDY_HPP
