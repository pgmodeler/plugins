//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file multiway_cut.hpp
 * @brief
 * @author Piotr Smulewicz, Piotr Godlewski
 * @version 1.0
 * @date 2013-12-19
 */

#ifndef PAAL_MULTIWAY_CUT_HPP
#define PAAL_MULTIWAY_CUT_HPP

#include "paal/lp/glp.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/irange.hpp"
#include "paal/utils/assign_updates.hpp"

#include <boost/bimap.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/as_array.hpp>

#include <fstream>
#include <tuple>
#include <utility>
#include <vector>
#include <random>


namespace paal {
namespace detail {

inline int vertices_column_index(int vertex, int dimentions, int column) {
    return vertex * dimentions + column;
}

template <class Graph>
using CostType = typename boost::property_traits<
    puretype(get(boost::edge_weight, std::declval<Graph>()))>::value_type;

template <typename LP> class multiway_cut_lp {
  public:
    /// Initialize the cut LP.
    template <typename Graph, typename IndexMap, typename WeightMap,
              typename ColorMap>
    void init(const Graph &graph, int k, const IndexMap &index_map,
              const WeightMap &weight_map, const ColorMap &color_map) {

        m_lp.set_lp_name("Multiway Cut");
        m_lp.set_optimization_type(lp::MINIMIZE);

        add_variables(graph, k, weight_map);
        add_constraints(graph, k, index_map, color_map);
    }

  private:
    // adding variables
    // returns the number of variables
    template <typename Graph, typename WeightMap>
    void add_variables(const Graph &graph, int k, const WeightMap &weight_map) {
        for (auto e : boost::as_array(edges(graph))) {
            for (int i = 0; i < k; ++i) {
                auto col_idx = m_lp.add_column(get(weight_map, e));
                edges_column.push_back(col_idx);
            }
        }
        for (unsigned vertex = 0; vertex <= num_vertices(graph); ++vertex) {
            for (int i = 0; i < k; ++i) {
                auto col_idx = m_lp.add_column(0);
                vertices_column.push_back(col_idx);
            }
        }
    }

    template <typename Graph, typename IndexMap, typename ColorMap>
    void add_constraints(const Graph &graph, int k, const IndexMap &index_map,
                         const ColorMap &color_map) {
        int db_index = 0;
        for (auto edge : boost::as_array(edges(graph))) {
            auto sour = get(index_map, source(edge, graph));
            auto targ = get(index_map, target(edge, graph));
            for (auto i : irange(k)) {
                for (auto j : irange(2)) {
                    auto x_e =
                        edges_column[vertices_column_index(db_index, k, i)];
                    auto x_src =
                        vertices_column[vertices_column_index(sour, k, i)];
                    auto x_trg =
                        vertices_column[vertices_column_index(targ, k, i)];
                    m_lp.add_row(
                        x_e + (j * 2 - 1) * x_src + (1 - 2 * j) * x_trg >= 0);
                }
            }
            ++db_index;
        }
        db_index = 0;
        for (auto vertex : boost::as_array(vertices(graph))) {
            auto col = get(color_map, vertex);
            if (col != 0) {
                auto x_col = vertices_column[
                    vertices_column_index(db_index, k, col - 1)];
                m_lp.add_row(x_col == 1);
            }
            lp::linear_expression expr;
            for (auto i : irange(k)) {
                expr += vertices_column[vertices_column_index(db_index, k, i)];
            }
            m_lp.add_row(std::move(expr) == 1);
            ++db_index;
        }
    }

  public:
    LP m_lp;
    std::vector<lp::col_id> edges_column;
    std::vector<lp::col_id> vertices_column;
};


template <typename Graph, typename VertexIndexMap, typename EdgeWeightMap,
          typename Dist, typename Rand, typename LP>
auto make_cut(const Graph &graph, int k, const VertexIndexMap &index_map,
              const EdgeWeightMap &weight_map, Dist &dist, Rand &&random_engine,
              multiway_cut_lp<LP> &mc_lp, std::vector<int> &vertex_to_part)
    ->detail::CostType<Graph> {
    double cut_cost = 0;
    std::vector<double> random_radiuses;
    dist.reset();
    for (int i = 0; i < k; ++i) {
        random_radiuses.push_back(dist(random_engine));
    }
    vertex_to_part.resize(num_vertices(graph));
    auto get_column = [&](int vertex, int dimension) {
        return mc_lp.m_lp.get_col_value(
            mc_lp.vertices_column[vertices_column_index(vertex, k, dimension)]);
    };

    for (auto vertex : boost::as_array(vertices(graph))) {
        for (int dimension = 0; dimension < k; ++dimension)
            if (1.0 - get_column(get(index_map, vertex), dimension) <
                    random_radiuses[dimension] ||
                dimension == k - 1) {
                // because each vertex have sum of coordinates equal 1,
                // 1.0-get_column(vertex,dimension) is proportional to distance
                // to vertex correspond to dimension
                vertex_to_part[get(index_map, vertex)] = dimension;
                break;
            }
    }
    for (auto edge : boost::as_array(edges(graph))) {
        if (vertex_to_part[get(index_map, source(edge, graph))] !=
            vertex_to_part[get(index_map, target(edge, graph))])
            cut_cost += get(weight_map, edge);
    }
    return cut_cost;
}


template <typename Rand = std::default_random_engine,
          typename Distribution = std::uniform_real_distribution<double>,
          typename LP = lp::glp, typename Graph, typename OutputIterator,
          typename VertexIndexMap, typename EdgeWeightMap,
          typename VertexColorMap>
auto multiway_cut_dispatch(const Graph &graph, OutputIterator result,
                           Rand &&random_engine, int iterations,
                           VertexIndexMap index_map, EdgeWeightMap weight_map,
                           VertexColorMap color_map)
    ->typename boost::property_traits<EdgeWeightMap>::value_type {
    using CostType = detail::CostType<Graph>;
    Distribution dis(0, 1);
    int terminals = 0;
    for (auto vertex : boost::as_array(vertices(graph))) {
        assign_max(terminals, get(color_map, vertex));
    }
    detail::multiway_cut_lp<LP> multiway_cut_lp;
    multiway_cut_lp.init(graph, terminals, index_map, weight_map, color_map);
    multiway_cut_lp.m_lp.solve_simplex(lp::DUAL);
    CostType cut_cost = std::numeric_limits<CostType>::max();
    std::vector<int> best_solution;
    std::vector<int> solution;
    for (int i = 0; i < iterations; ++i) {
        solution.clear();
        int res = detail::make_cut(graph, terminals, index_map, weight_map, dis,
                                   random_engine, multiway_cut_lp, solution);
        if (res < cut_cost) {
            swap(solution, best_solution);
            cut_cost = res;
        }
    }
    for (auto v : boost::as_array(vertices(graph))) {
        *result = std::make_pair(v, best_solution[get(index_map, v)]);
        ++result;
    }
    return cut_cost;
}

template <typename Rand = std::default_random_engine,
          typename Distribution = std::uniform_real_distribution<double>,
          typename LP = lp::glp, typename Graph, typename OutputIterator,
          typename VertexIndexMap, typename EdgeWeightMap,
          typename VertexColorMap>
auto multiway_cut_dispatch(const Graph &graph, OutputIterator result,
                           Rand &&random_engine, boost::param_not_found,
                           VertexIndexMap index_map, EdgeWeightMap weight_map,
                           VertexColorMap color_map)
    ->typename boost::property_traits<EdgeWeightMap>::value_type {
    int vertices = num_vertices(graph);
    const static int MIN_NUMBER_OF_REPEATS = 100;
    auto number_of_repeats =
        vertices * vertices +
        MIN_NUMBER_OF_REPEATS; // This variable is not supported by any proof
    return multiway_cut_dispatch(graph, result, random_engine,
                                 number_of_repeats, index_map, weight_map,
                                 color_map);
}

} //!detail

/**
 * @brief this is solve multiway_cut problem
 * and return cut_cost
 * example:
 *  \snippet multiway_cut_example.cpp  Multiway Cut Example
 * @param Graph graph
 * @param OutputIterator result pairs of vertex descriptor and number form (1,2,
 * ... ,k) id of part
 * @param random_engine
 * @param params
 * @tparam Graph
 * @tparam OutputIterator
 * @tparam Rand random engine
 * @tparam Distribution used to chose random radius
 * @tparam LP
 * @tparam P
 * @tparam T
 * @tparam R
 */
template <typename Rand = std::default_random_engine,
          typename Distribution = std::uniform_real_distribution<double>,
          typename LP = lp::glp, typename Graph, typename OutputIterator,
          typename P, typename T, typename R>
auto multiway_cut(const Graph &g, OutputIterator out,
                  const boost::bgl_named_params<P, T, R> &params,
                  Rand &&random_engine = std::default_random_engine(5426u))
    ->typename boost::property_traits<puretype(
          boost::choose_const_pmap(get_param(params, boost::edge_weight), g,
                                   boost::edge_weight))>::value_type {
    return detail::multiway_cut_dispatch(
        g, out, random_engine, get_param(params, boost::iterations_t()),
        boost::choose_const_pmap(get_param(params, boost::vertex_index), g,
                                 boost::vertex_index),
        boost::choose_const_pmap(get_param(params, boost::edge_weight), g,
                                 boost::edge_weight),
        boost::choose_const_pmap(get_param(params, boost::vertex_color), g,
                                 boost::vertex_color));
}

/**
 * @brief this is solve multiway_cut problem
 * and return cut_cost
 * example:
 *  \snippet multiway_cut_example.cpp  Multiway Cut Example
 *
 * example file is  multiway_cut_example.cpp
 * @param Graph graph
 * @param int repeats number of sets of radius
 * @param OutputIterator result pairs of vertex descriptor and number form (1,2,
* ... ,k) id of part
 * @tparam Rand random engine
 * @tparam Distribution used to chose random radius
 * @tparam LP
 * @tparam Graph
 * @tparam OutputIterator
 */
template <typename Rand = std::default_random_engine,
          typename Distribution = std::uniform_real_distribution<double>,
          typename LP = lp::glp, typename Graph, class OutputIterator>
auto multiway_cut(const Graph &graph, OutputIterator result,
                  Rand random_engine = std::default_random_engine(5426u))
    ->detail::CostType<Graph> {
    return multiway_cut(graph, result, boost::no_named_parameters(),
                        random_engine);
}

}      //!paal
#endif // PAAL_MULTIWAY_CUT_HPP
