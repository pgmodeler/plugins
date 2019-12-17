//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file graph_metrics.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
#ifndef PAAL_GRAPH_METRICS_HPP
#define PAAL_GRAPH_METRICS_HPP

#include "basic_metrics.hpp"

#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/adjacency_matrix.hpp>

namespace paal {
namespace data_structures {

namespace graph_type {
class sparse_tag;
class dense_tag;
class large_tag;
}

/**
 * @brief traits for graph metric
 *
 * @tparam Graph
 */
template <typename Graph> struct graph_metric_traits {
    //default graph_type
    using graph_tag_type = graph_type::sparse_tag;
};


/// generic strategies of computing metric
template <typename graph_tag_type> struct graph_metric_filler_impl;

/**
 * @brief specialization for sparse_tag graphs
 */
template <> struct graph_metric_filler_impl<graph_type::sparse_tag> {
    /**
     * @brief fill_matrix function
     *
     * @tparam Graph
     * @tparam ResultMatrix
     * @param g
     * @param rm
     */
    template <typename Graph, typename ResultMatrix>
    void fill_matrix(const Graph &g, ResultMatrix &rm) {
        boost::johnson_all_pairs_shortest_paths(g, rm);
    }
};

/**
 * @brief specialization strategies of computing metric for dense_tag graphs
 */
template <> struct graph_metric_filler_impl<graph_type::dense_tag> {
    template <typename Graph, typename ResultMatrix>
    /**
     * @brief fill_matrixFunction
     *
     * @param g
     * @param rm
     */
        void fill_matrix(const Graph &g, ResultMatrix &rm) {
        boost::floyd_warshall_all_pairs_shortest_paths(g, rm);
    }
};

/**
 * @class graph_metric
 * @brief Adopts boost graph as \ref metric.
 *
 * @tparam Graph
 * @tparam DistanceType
 * @tparam GraphType
 */
// GENERIC
// GraphType could be sparse, dense, large ...
template <
    typename Graph, typename DistanceType,
    typename GraphType = typename graph_metric_traits<Graph>::graph_tag_type>
struct graph_metric : public array_metric<DistanceType>,
                      public graph_metric_filler_impl<
                          typename graph_metric_traits<Graph>::graph_tag_type> {
    typedef array_metric<DistanceType> GMBase;
    typedef graph_metric_filler_impl<
        typename graph_metric_traits<Graph>::graph_tag_type> GMFBase;

    /**
     * @brief constructor
     *
     * @param g
     */
    graph_metric(const Graph &g) : GMBase(num_vertices(g)) {
        GMFBase::fill_matrix(g, GMBase::m_matrix);
    }
};

// TODO implement
/// Specialization for large graphs
template <typename Graph, typename DistanceType>
struct graph_metric<Graph, DistanceType, graph_type::large_tag> {
    /**
     * @brief constructor
     *
     * @param g
     */
    graph_metric(const Graph &g) { assert(false); }
};

/// Specialization for adjacency_list
template <typename OutEdgeList, typename VertexList, typename Directed,
          typename VertexProperties, typename EdgeProperties,
          typename GraphProperties, typename EdgeList>
struct graph_metric_traits<
    boost::adjacency_list<OutEdgeList, VertexList, Directed, VertexProperties,
                          EdgeProperties, GraphProperties, EdgeList>> {
    typedef graph_type::sparse_tag graph_tag_type;
};

/// Specialization for adjacency_matrix
template <typename Directed, typename VertexProperty, typename EdgeProperty,
          typename GraphProperty, typename Allocator>
struct graph_metric_traits<boost::adjacency_matrix<
    Directed, VertexProperty, EdgeProperty, GraphProperty, Allocator>> {
    typedef graph_type::dense_tag graph_tag_type;
};

} //!data_structures
} //!paal

#endif // PAAL_GRAPH_METRICS_HPP
