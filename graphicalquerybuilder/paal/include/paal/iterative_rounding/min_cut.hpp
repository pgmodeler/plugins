//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file min_cut.hpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2014-03-05
 */
#ifndef PAAL_MIN_CUT_HPP
#define PAAL_MIN_CUT_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>

#include <numeric>

namespace paal {
namespace ir {

/**
 * @class min_cut_finder
 * @brief Class for creating and modifying directed graphs with edge capacities
 *  and finding directed minimum cuts between given vertices.
 */
class min_cut_finder {
    using Traits = boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS>;
public:
    using Edge = Traits::edge_descriptor;
    using Vertex = Traits::vertex_descriptor;

    /// Constructor.
    min_cut_finder()
        : m_graph(0), m_cap(get(boost::edge_capacity, m_graph)),
          m_rev(get(boost::edge_reverse, m_graph)),
          m_colors(get(boost::vertex_color, m_graph)) {}

    /**
     * (Re)Initializes the graph.
     */
    void init(int vertices_num) {
        m_graph.clear();
        for (int i = 0; i < vertices_num; ++i) {
            add_vertex_to_graph();
        }
        m_cap = get(boost::edge_capacity, m_graph);
        m_rev = get(boost::edge_reverse, m_graph);
    }

    /**
     * Adds a new vertex to the graph.
     */
    Vertex add_vertex_to_graph() { return add_vertex(m_graph); }

    /**
     * Adds an edge to the graph.
     * @param src source vertex of for the added edge
     * @param trg target vertex of for the added edge
     * @param cap capacity of the added edge
     * @param rev_cap capacity of the reverse edge
     *
     * @return created edge of the graph and the created reverse edge
     */
    std::pair<Edge, Edge>
    add_edge_to_graph(Vertex src, Vertex trg, double cap, double rev_cap = 0.) {
        bool b, b_rev;
        Edge e, e_rev;

        std::tie(e, b) = add_edge(src, trg, m_graph);
        std::tie(e_rev, b_rev) = add_edge(trg, src, m_graph);
        assert(b && b_rev);

        put(m_cap, e, cap);
        put(m_cap, e_rev, rev_cap);

        put(m_rev, e, e_rev);
        put(m_rev, e_rev, e);

        return std::make_pair(e, e_rev);
    }

    /**
     * Finds the min cut between \c src and \c trg.
     *
     * @param src source vertex (belongs to the cut set)
     * @param trg target vertex (does not belong to the cut set)
     *
     * @return min cut value
     */
    double find_min_cut(Vertex src, Vertex trg) {
        assert(src != trg);
        double min_cut_val = boost::boykov_kolmogorov_max_flow(m_graph, src, trg);
        m_colors = get(boost::vertex_color, m_graph);
        m_src_color = get(m_colors, src);
        m_last_cut = std::make_pair(src, trg);
        assert(!is_in_source_set(trg));
        return min_cut_val;
    }

    /**
     * Checks if the given vertex belongs to the source side of the last checked
     * cut.
     */
    bool is_in_source_set(Vertex v) const {
        return (m_src_color == get(m_colors, v));
    }

    /**
     * Returns the number of vertices in the source size of the last checked
     * cut.
     */
    int source_set_size() const {
        auto verts = vertices(m_graph);
        return std::accumulate(verts.first, verts.second, 0,
                               [&](int count, Vertex v) {
            return count + is_in_source_set(v);
        });
    }

    /**
     * Returns the pair of vertices defining the last checked cut.
     */
    std::pair<Vertex, Vertex> get_last_cut() const { return m_last_cut; }

    /**
     * Returns the capacity of a given edge.
     */
    double get_capacity(Edge e) const { return get(m_cap, e); }

    /**
     * Sets the capacity of a given edge.
     */
    void set_capacity(Edge e, double cap) {
        put(m_cap, e, cap);
    }

    /**
     * Sets the capacity of a given edge.
     */
    void set_capacity(Vertex src, Vertex trg, double cap) {
        auto e = edge(src, trg, m_graph);
        assert(e.second);
        set_capacity(e.first, cap);
    }

private:
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
              boost::property<boost::vertex_color_t, boost::default_color_type,
                  boost::property<boost::vertex_distance_t, long,
                      boost::property<boost::vertex_predecessor_t, Edge>>>,
              boost::property<boost::edge_capacity_t, double,
                  boost::property<boost::edge_residual_capacity_t, double,
                      boost::property<boost::edge_reverse_t, Edge>>>>;
    using EdgeCapacity = boost::property_map<Graph, boost::edge_capacity_t>::type;
    using EdgeReverse = boost::property_map<Graph, boost::edge_reverse_t>::type;
    using VertexColors = boost::property_map<Graph, boost::vertex_color_t>::type;

    Graph m_graph;

    EdgeCapacity m_cap;
    EdgeReverse m_rev;
    VertexColors m_colors;

    boost::default_color_type m_src_color;
    std::pair<Vertex, Vertex> m_last_cut;
};

} //! ir
} //! paal
#endif // PAAL_MIN_CUT_HPP
