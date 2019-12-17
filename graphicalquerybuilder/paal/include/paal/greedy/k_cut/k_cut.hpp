//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file k_cut.hpp
 * @brief
 * @author Piotr Smulewicz, Piotr Godlewski
 * @version 1.0
 * @date 2013-09-25
 */
#ifndef PAAL_K_CUT_HPP
#define PAAL_K_CUT_HPP

#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/irange.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/range/as_array.hpp>

#include <queue>

namespace paal {
namespace greedy {

/**
 * @brief this is solve k_cut problem
 * and return cut_cost
 * example:
 *  \snippet k_cut_example.cpp K Cut Example
 *
 * example file is k_cut_example.cpp
 * @param graph
 * @param number_of_parts
 * @param result pairs of vertex_descriptor and number form (1,2,
* ... ,k) id of part
 * @param index_map
 * @param weight_map
 * @tparam InGraph
 * @tparam OutputIterator
 * @tparam VertexIndexMap
 * @tparam EdgeWeightMap
 */
template<typename InGraph, class OutputIterator, typename VertexIndexMap, typename EdgeWeightMap>
auto k_cut(const InGraph& graph, unsigned int number_of_parts,OutputIterator result,
            VertexIndexMap index_map, EdgeWeightMap weight_map) ->
            typename boost::property_traits<EdgeWeightMap>::value_type{
    using cost_t = typename boost::property_traits<EdgeWeightMap>::value_type;
    using Vertex = typename boost::graph_traits<InGraph>::vertex_descriptor;

    using Graph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::undirectedS, boost::no_property,
        boost::property<boost::edge_weight_t, cost_t,
                        boost::property<boost::edge_index_t, int>>>;

    assert(num_vertices(graph) >= number_of_parts);

    std::vector<int> vertex_to_part(num_vertices(graph));
    using VertexIndexToVertex = typename std::vector<Vertex>;
    using VertexIndexToVertexIndex = std::vector<int>;
    VertexIndexToVertex vertex_in_subgraph_to_vertex(num_vertices(graph));
    VertexIndexToVertexIndex vertex_to_vertex_in_subgraph(num_vertices(graph));
    int vertex_in_part;
    int parts = 1;
    // cuts contain pair(x,y)
    // x is the cost of the cut
    // y and y+1 are index parts of graph after make a cut
    std::priority_queue<
            std::pair<cost_t,int>,
            std::vector<std::pair<cost_t,int> >
            ,utils::greater> cuts;

    int id_part = 0;

    //get part id and compute minimum cost of cut of that part and add it to queue
    auto make_cut = [&](int id) {
        vertex_in_part=0;
        for (auto v: boost::as_array(vertices(graph))) {
            if (vertex_to_part[get(index_map, v)] == id) {
                vertex_in_subgraph_to_vertex[vertex_in_part] = v;
                vertex_to_vertex_in_subgraph[get(index_map, v)] = vertex_in_part;
                ++vertex_in_part;
            }
        }
        Graph part(vertex_in_part);
        for (auto edge : boost::as_array(edges(graph))) {
            auto sour = get(index_map, source(edge,graph));
            auto targ = get(index_map, target(edge,graph));
            if (vertex_to_part[sour] == id &&
                    vertex_to_part[targ] == id &&
                    sour != targ) {
                add_edge(vertex_to_vertex_in_subgraph[sour],
                         vertex_to_vertex_in_subgraph[targ],
                         get(weight_map, edge),
                         part);
            }
        }
        if (vertex_in_part < 2) {
            ++id_part;
            *result = std::make_pair(vertex_in_subgraph_to_vertex[0], id_part);
            ++result;
            return;
        }
        auto parities = boost::make_one_bit_color_map(num_vertices(part),
                                            get(boost::vertex_index, part));
        auto cut_cost = boost::stoer_wagner_min_cut(part,
                                          get(boost::edge_weight, part),
                                          boost::parity_map(parities));

        for (auto i : irange(num_vertices(part))) {
            vertex_to_part[get(index_map, vertex_in_subgraph_to_vertex[i])] =
                    parts + get(parities, i); //return value convertable to 0/1
        }
        cuts.push(std::make_pair(cut_cost, parts));
        parts += 2;
    };

    make_cut(0);
    cost_t k_cut_cost = cost_t();
    while (--number_of_parts) {
        auto cut = cuts.top();
        cuts.pop();
        k_cut_cost += cut.first;
        make_cut(cut.second);
        make_cut(cut.second + 1);
    }

    while (!cuts.empty()) {
        auto cut = cuts.top();
        cuts.pop();
        ++id_part;
        for (auto v: boost::as_array(vertices(graph))) {
            if (vertex_to_part[get(index_map, v)] == cut.second ||
                    vertex_to_part[get(index_map, v)] == cut.second + 1) {
                *result = std::make_pair(v, id_part);
                ++result;
            }
        }
    }
    return k_cut_cost;
}

/**
 * @brief this is solve k_cut problem
 * and return cut_cost
 * example:
 *  \snippet k_cut_example.cpp K Cut Example
 *
 * example file is k_cut_example.cpp
 * @param graph
 * @param number_of_parts
 * @param result pairs of vertex_descriptor and number form (1,2, ... ,k) id of part
 * @param params
 * @tparam InGraph
 * @tparam OutputIterator
 * @tparam T
 * @tparam P
 * @tparam R
 */
template<typename InGraph
        ,class OutputIterator
        ,typename T
        ,typename P
        ,typename R>
auto k_cut(const InGraph& graph, unsigned int number_of_parts,
    OutputIterator result, const boost::bgl_named_params<P, T, R>& params) ->
        typename boost::property_traits<
            puretype(boost::choose_const_pmap(get_param(params, boost::edge_weight), graph, boost::edge_weight))
            >::value_type {
    return k_cut(graph, number_of_parts, result,
        boost::choose_const_pmap(get_param(params, boost::vertex_index), graph,boost::vertex_index),
        boost::choose_const_pmap(get_param(params, boost::edge_weight), graph,boost::edge_weight)
    );
}

/**
 * @brief this is solve k_cut problem
 * and return cut_cost
 * example:
 *  \snippet k_cut_example.cpp K Cut Example
 *
 * example file is k_cut_example.cpp
 * @param graph
 * @param number_of_parts
 * @param result pairs of vertex_descriptor and number form (1,2, ... ,k) id of part
 * @tparam InGraph
 * @tparam OutputIterator
 */
template<typename InGraph, class OutputIterator>
auto k_cut(const InGraph& graph, unsigned int number_of_parts, OutputIterator result) ->
        typename boost::property_traits<puretype(get(boost::edge_weight,graph))>::value_type{
    return k_cut(graph, number_of_parts, result, boost::no_named_parameters());
}

} //!greedy
} //!paal

#endif // PAAL_K_CUT_HPP
