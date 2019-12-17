//=======================================================================
// Copyright (c) 2013 Attila Bernath, Piotr Wygocki
//               2014 Piotr Godlewski, Piotr Smulewicz
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file tree_augmentation.hpp
 * @brief
 * @author Attila Bernath, Piotr Smulewicz, Piotr Wygocki, Piotr Godlewski
 * @version 1.0
 * @date 2013-06-20
 */
#ifndef PAAL_TREE_AUGMENTATION_HPP
#define PAAL_TREE_AUGMENTATION_HPP


#include "paal/iterative_rounding/ir_components.hpp"
#include "paal/iterative_rounding/iterative_rounding.hpp"
#include "paal/utils/hash.hpp"

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
#include <boost/range/distance.hpp>

#include <algorithm>
#include <utility>

namespace paal {
namespace ir {

namespace detail {

/// This function returns the number of edges in a graph. The
/// reason this function was written is that BGL's num_edges()
/// function does not work properly for filtered_graph.  See
/// http://www.boost.org/doc/libs/1_55_0/libs/graph/doc/filtered_graph.html#2
template <class EdgeListGraph>
int filtered_num_edges(const EdgeListGraph & g) {
    return boost::distance(edges(g));
}

/// A class that translates bool map on the edges to a filter, which can be used
/// with
/// boost::filtered_graph. That is, we translate operator[] to
/// operator().  We do a little more: we will also need the negated
/// version of the map (for the graph of the non-tree edges).
template <typename EdgeBoolMap> struct bool_map_to_tree_filter {
    bool_map_to_tree_filter() {}

    bool_map_to_tree_filter(EdgeBoolMap m) : ebmap(m) {}

    template <typename Edge> bool operator()(const Edge &e) const {
        return get(ebmap, e);
    }

    EdgeBoolMap ebmap;
};

template <typename EdgeBoolMap> struct bool_map_to_non_tree_filter {
    bool_map_to_non_tree_filter() {}

    bool_map_to_non_tree_filter(EdgeBoolMap m) : ebmap(m) {}

    template <typename Edge> bool operator()(const Edge &e) const {
        return !get(ebmap, e);
    }

    EdgeBoolMap ebmap;
};

/// A boost graph map that returns a constant integer value.
template <typename KeyType, int num>
class const_int_map
    : public boost::put_get_helper<int, const_int_map<KeyType, num>> {
  public:
    using category = boost::readable_property_map_tag;
    using value_type = int;
    using reference = int;
    using key_type = KeyType;
    reference operator[](KeyType e) const { return num; }
};

} // namespace detail

namespace {
struct ta_compare_traits {
    static const double EPSILON;
};

const double ta_compare_traits::EPSILON = 1e-10;
}

/**
 * Round Condition of the IR Tree Augmentation algorithm.
 */
struct ta_round_condition {
    /**
     * Constructor. Takes epsilon used in double comparison.
     */
    ta_round_condition(double epsilon = ta_compare_traits::EPSILON)
        : m_round_half(epsilon) {}

    /**
     * Rounds a column up if it is at least half in the
     * current solution. If the column is rounded, the
     * corresponding edge is added to the result.
     */
    template <typename Problem, typename LP>
    boost::optional<double> operator()(Problem &problem, LP &lp,
                                       lp::col_id col) {
        auto res = m_round_half(problem, lp, col);
        if (res) {
            problem.add_to_solution(col);
        }
        return res;
    }

  private:
    round_condition_greater_than_half m_round_half;
};

/**
 * Set Solution component of the IR Tree Augmentation algorithm.
 */
struct ta_set_solution {
    /**
     * Constructor. Takes epsilon used in double comparison.
     */
    ta_set_solution(double epsilon = ta_compare_traits::EPSILON)
        : m_compare(epsilon) {}

    /**
     * Creates the result form the LP (all edges corresponding to columns with
     * value 1).
     */
    template <typename Problem, typename GetSolution>
    void operator()(Problem &problem, const GetSolution &solution) {
        for (auto e :
             boost::as_array(edges(problem.get_links_graph()))) {
            if (!problem.is_in_solution(e)) {
                auto col = problem.edge_to_col(e);
                if (m_compare.e(solution(col), 1)) {
                    problem.add_to_solution(col);
                }
            }
        }
    }

private:
    const utils::compare<double> m_compare;
};

/**
 * Initialization of the IR Tree Augmentation algorithm.
 */
class ta_init {
  public:
    /**
     * Initialize the cut LP.
     */
    template <typename Problem, typename LP>
    void operator()(Problem &problem, LP &lp) {
        problem.init();
        lp.set_lp_name("Tree augmentation");
        lp.set_optimization_type(lp::MINIMIZE);

        add_variables(problem, lp);
        add_cut_constraints(problem, lp);
    }

  private:
    /**
     * Adds a variable to the LP for each link in the input graph.
     * Binds the LP columns to graph edges.
     */
    template <typename Problem, typename LP>
    void add_variables(Problem & problem, LP & lp) {
        for (auto e : boost::as_array(edges(problem.get_links_graph()))) {
            lp::col_id col_idx = lp.add_column(problem.get_cost(e), 0);
            problem.bind_edge_to_col(e, col_idx);
        }
    }

    /**
     * Adds a cut constraint to the LP for each edge in the input graph
     * and binds edges to LP rows.
     */
    template <typename Problem, typename LP>
    void add_cut_constraints(Problem &problem, LP &lp) {
        for (auto e :
             boost::as_array(edges(problem.get_tree_graph()))) {
            lp::linear_expression expr;
            for (auto pe : problem.get_covered_by(e)) {
                expr += problem.edge_to_col(pe);
            }

            lp::row_id row_idx = lp.add_row(std::move(expr) >= 1);
            problem.bind_edge_to_row(e, row_idx);
        }
    }
};

template <
    typename Init = ta_init,
    typename RoundCondition = ta_round_condition,
    typename RelaxContition = utils::always_false,
    typename SetSolution = ta_set_solution>
        using tree_augmentation_ir_components = IRcomponents<Init, RoundCondition,
                        RelaxContition, SetSolution>;

/**
 * @brief This is Jain's iterative rounding
 * 2-approximation algorithm for the Generalised Steiner Network
 * Problem, specialized for the Tree Augmentation Problem.
 *
 * The Tree Augmentation Problem is the following. Given a
 * 2-edge connected graph, in which a spanning tree is
 * designated. The non-tree edges are also called links. The
 * links have non-negative costs. The problem is to find a
 * minimum cost subset of links which, together with the
 * tree-edges give a 2-edge-connected graph.
 *
 * @tparam Graph the graph type used
 * @tparam TreeMap it is assumed to be a bool map on the edges of a graph of type Graph.
 *  It is used for designating a spanning tree in the graph.
 * @tparam CostMap type for the costs of the links.
 * @tparam VertexIndex type for the vertex index map.
 * @tparam EdgeSetOutputIterator type for the result edge set.
 */
template <typename Graph, typename TreeMap, typename CostMap,
          typename VertexIndex, typename EdgeSetOutputIterator>
class tree_aug {
  public:
    using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    using CostValue = double;

    using TreeGraph =
        boost::filtered_graph<Graph, detail::bool_map_to_tree_filter<TreeMap>>;
    using NonTreeGraph = boost::filtered_graph<
        Graph, detail::bool_map_to_non_tree_filter<TreeMap>>;

    using EdgeList = std::vector<Edge>;
    using CoverMap = std::unordered_map<Edge, EdgeList, edge_hash<Graph>>;

    // cross reference between links and columns
    using EdgeToColId = boost::bimap<Edge, lp::col_id>;
    using RowIdToEdge = std::unordered_map<lp::row_id, Edge>;

    using ErrorMessage = boost::optional<std::string>;

    /**
     * Constructor.
     *
     * @param g  the graph to work with
     * @param tree_map designate a spanning tree in \c g
     * @param cost_map costs of the links (=non-tree edges). The costs assigned to tree edges are not used.
     * @param vertex_index vertex index map
     * @param solution result set of edges output iterator
     */
    tree_aug(const Graph & g, TreeMap tree_map, CostMap cost_map,
            VertexIndex vertex_index, EdgeSetOutputIterator solution) :
        m_g(g), m_tree_map(tree_map), m_cost_map(cost_map), m_index(vertex_index),
        m_solution(solution),
        m_tree(m_g, detail::bool_map_to_tree_filter<TreeMap>(m_tree_map)),
        m_ntree(m_g, detail::bool_map_to_non_tree_filter<TreeMap>(m_tree_map)),
        m_sol_cost(0)
    {}

    /// Checks validity of the input
    ErrorMessage check_input_validity() {
        // Num of edges == num of nodes-1 in the tree?
        int n_v = num_vertices(m_g);
        int n_e = filtered_num_edges(m_tree);

        if (n_e != n_v - 1) {
            return "Incorrect number of edges in the spanning tree. "
                        + std::string("Should be ") + std::to_string(n_v - 1)
                        + ", but it is " + std::to_string(n_e) + ".";
        }

        // Is the tree connected?
        std::vector<int> component(num_vertices(m_g));
        int num = boost::connected_components(m_tree, &component[0]);
        if (num > 1) {
            return ErrorMessage{ "The spanning tree is not connected." };
        }

        // Is the graph 2-edge-connected?
        detail::const_int_map<Edge, 1> const_1_edge_map;
        // TODO This stoer-wagner algorithm is unnecessarily slow for some reason
        int min_cut = boost::stoer_wagner_min_cut(m_g, const_1_edge_map);
        if (min_cut < 2) {
            return ErrorMessage{"The graph is not 2-edge-connected."};
        }

        return ErrorMessage{};
    }

    /**
     * Returns the non-tree graph (set of links).
     */
    const NonTreeGraph &get_links_graph() const { return m_ntree; }

    /**
     * Returns the spanning tree.
     */
    const TreeGraph &get_tree_graph() const { return m_tree; }

    /**
     * Returns the cost of an edge.
     */
    auto get_cost(Edge e)->decltype(get(std::declval<CostMap>(), e)) {
        return get(m_cost_map, e);
    }

    /**
     * Adds an edge corresponding to the given LP column to the result set.
     */
    void add_to_solution(lp::col_id col) {
        *m_solution = m_edge_to_col_id.right.at(col);
        ++m_solution;
        m_sol_cost += m_cost_map[m_edge_to_col_id.right.at(col)];
        m_edge_to_col_id.right.erase(col);
    }

    /**
     * Binds a graph edge to a LP column.
     */
    void bind_edge_to_col(Edge e, lp::col_id col) {
        auto tmp =
            m_edge_to_col_id.insert(typename EdgeToColId::value_type(e, col));
        assert(tmp.second);
    }

    /**
     * Binds a graph edge to a LP row.
     */
    void bind_edge_to_row(Edge e, lp::row_id row) {
        auto tmp =
            m_row_id_to_edge.insert(typename RowIdToEdge::value_type(row, e));
        assert(tmp.second);
    }

    /**
     * Initializes the necessary data structures.
     */
    void init() {
        // We need to fill very useful auxiliary data structures:
        //\c m_covered_by - containing lists of
        // edges. For a tree edge \c t the list \c m_covered_by[t]
        // contains the list of links covering \c t.

        std::vector<Edge> pred(num_vertices(m_g));
        auto pred_map =
            boost::make_iterator_property_map(pred.begin(), m_index);
        std::set<Vertex> seen;
        for (auto u : boost::as_array(vertices(m_g))) {
            auto tmp = seen.insert(u);
            assert(tmp.second);
            boost::breadth_first_search(
                m_tree, u, boost::visitor(boost::make_bfs_visitor(
                               boost::record_edge_predecessors(
                                   pred_map, boost::on_tree_edge()))));

            for (auto e : boost::as_array(out_edges(u, m_ntree))) {
                auto node = target(e, m_ntree);
                if (!seen.count(node)) {
                    while (node != u) {
                        m_covered_by[get(pred_map, node)].push_back(e);
                        node = source(get(pred_map, node), m_tree);
                    }
                }
            }
        }
    }

    /**
     * Returns the edge corresponding to an LP row.
     */
    Edge row_to_edge(lp::row_id row) const { return m_row_id_to_edge.at(row); }

    /**
     * Returns the LP columnn corresponding to a graph edge.
     */
    lp::col_id edge_to_col(Edge e) const { return m_edge_to_col_id.left.at(e); }

    /**
     * Returns the list of links covering a given edge.
     */
    EdgeList &get_covered_by(Edge e) { return m_covered_by[e]; }

    /**
     * Checks if an edge belongs to the solution.
     */
    bool is_in_solution(Edge e) const {
        return m_edge_to_col_id.left.find(e) == m_edge_to_col_id.left.end();
    }

    /**
     * Returns cost of the found solution.
     */
    CostValue get_solution_cost() const { return m_sol_cost; }

  private:

    /// Input graph
    const Graph &m_g;
    /// Input tree edges map
    TreeMap m_tree_map;
    /// Input edge cost map
    CostMap m_cost_map;
    /// Vertex index map
    VertexIndex m_index;

    /// Which links are chosen in the solution
    EdgeSetOutputIterator m_solution;

    /// Auxiliary data structures
    EdgeToColId m_edge_to_col_id;

    /// The spanning tree
    TreeGraph m_tree;
    /// The non-tree (=set of links)
    NonTreeGraph m_ntree;
    /// Cost of the solution found
    CostValue m_sol_cost;
    /// Structures for the "m_covered_by" relations
    CoverMap m_covered_by;
    /// Reference between tree edges and row ids
    RowIdToEdge m_row_id_to_edge;
};

namespace detail {
/**
 * @brief Creates a tree_aug object. Non-named parameters.
 *
 * @tparam Graph
 * @tparam TreeMap
 * @tparam CostMap
 * @tparam VertexIndex
 * @tparam EdgeSetOutputIterator
 * @param g
 * @param tree_map
 * @param cost_map
 * @param vertex_index
 * @param solution
 *
 * @return tree_aug object
 */
template <typename Graph, typename TreeMap, typename CostMap,
          typename VertexIndex, typename EdgeSetOutputIterator>
tree_aug<Graph, TreeMap, CostMap, VertexIndex, EdgeSetOutputIterator>
make_tree_aug(const Graph & g, TreeMap tree_map, CostMap cost_map,
                VertexIndex vertex_index, EdgeSetOutputIterator solution) {
    return paal::ir::tree_aug<Graph, TreeMap, CostMap,
            VertexIndex, EdgeSetOutputIterator>(g, tree_map, cost_map, vertex_index, solution);
}

/**
 * @brief Solves the Tree Augmentation problem using Iterative Rounding.
* Non-named parameters.
 *
 * @tparam Graph
 * @tparam TreeMap
 * @tparam CostMap
 * @tparam VertexIndex
 * @tparam EdgeSetOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @param g
 * @param tree_map
 * @param cost_map
 * @param vertex_index
 * @param solution
 * @param components
 * @param visitor
 *
 * @return solution status
 */
template <typename Graph, typename TreeMap, typename CostMap,
          typename VertexIndex, typename EdgeSetOutputIterator,
          typename IRcomponents = tree_augmentation_ir_components<>,
          typename Visitor = trivial_visitor>
IRResult tree_augmentation_iterative_rounding(
        const Graph & g,
        TreeMap tree_map,
        CostMap cost_map,
        VertexIndex vertex_index,
        EdgeSetOutputIterator solution,
        IRcomponents components = IRcomponents(),
        Visitor visitor = Visitor()) {
    auto treeaug = make_tree_aug(g, tree_map, cost_map, vertex_index, solution);
    return solve_iterative_rounding(treeaug, std::move(components), std::move(visitor));
}
} // detail

/**
 * Creates a tree_aug object. Named parameters.
 * The returned object can be used to check input validity or to get a lower
* bound on the
 * optimal solution cost.
 *
 * @tparam Graph
 * @tparam EdgeSetOutputIterator
 * @tparam P
 * @tparam T
 * @tparam R
 * @param g
 * @param params
 * @param solution
 *
 * @return tree_aug object
 */
template <typename Graph, typename EdgeSetOutputIterator, typename P,
          typename T, typename R>
auto make_tree_aug(const Graph &g,
                   const boost::bgl_named_params<P, T, R> &params,
                   EdgeSetOutputIterator solution)
    ->tree_aug<
          Graph,
          decltype(choose_const_pmap(get_param(params, boost::edge_color), g,
                                     boost::edge_color)),
          decltype(choose_const_pmap(get_param(params, boost::edge_weight), g,
                                     boost::edge_weight)),
          decltype(choose_const_pmap(get_param(params, boost::vertex_index), g,
                                     boost::vertex_index)),
          EdgeSetOutputIterator> {
    return detail::make_tree_aug(
        g, choose_const_pmap(get_param(params, boost::edge_color), g,
                             boost::edge_color),
        choose_const_pmap(get_param(params, boost::edge_weight), g,
                          boost::edge_weight),
        choose_const_pmap(get_param(params, boost::vertex_index), g,
                          boost::vertex_index),
        solution);
}

/**
 * Creates a tree_aug object. All default parameters.
 * The returned object can be used to check input validity or to get a lower
* bound on the
 * optimal solution cost.
 *
 * @tparam Graph
 * @tparam EdgeSetOutputIterator
 * @param g
 * @param solution
 *
 * @return tree_aug object
 */
template <typename Graph, typename EdgeSetOutputIterator>
auto make_tree_aug(const Graph &g, EdgeSetOutputIterator solution)
    ->decltype(make_tree_aug(g, boost::no_named_parameters(), solution)) {
    return make_tree_aug(g, boost::no_named_parameters(), solution);
}

/**
 * @brief Solves the Tree Augmentation problem using Iterative Rounding. Named
* parameters.
 *
 * @tparam Graph
 * @tparam EdgeSetOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @tparam P
 * @tparam T
 * @tparam R
 * @param g
 * @param params
 * @param solution
 * @param components
 * @param visitor
 *
 * @return solution status
 */
template <typename Graph, typename EdgeSetOutputIterator,
          typename IRcomponents = tree_augmentation_ir_components<>,
          typename Visitor = trivial_visitor, typename P, typename T,
          typename R>
IRResult tree_augmentation_iterative_rounding(
    const Graph &g, const boost::bgl_named_params<P, T, R> &params,
    EdgeSetOutputIterator solution, IRcomponents components = IRcomponents(),
    Visitor visitor = Visitor()) {
    return detail::tree_augmentation_iterative_rounding(
        g, choose_const_pmap(get_param(params, boost::edge_color), g,
                             boost::edge_color),
        choose_const_pmap(get_param(params, boost::edge_weight), g,
                          boost::edge_weight),
        choose_const_pmap(get_param(params, boost::vertex_index), g,
                          boost::vertex_index),
        std::move(solution), std::move(components), std::move(visitor));
}

/**
 * @brief Solves the Tree Augmentation problem using Iterative Rounding. All
* default parameters.
 *
 * @tparam Graph
 * @tparam EdgeSetOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @param g
 * @param solution
 * @param components
 * @param visitor
 *
 * @return solution status
 */
template <typename Graph, typename EdgeSetOutputIterator,
          typename IRcomponents = tree_augmentation_ir_components<>,
          typename Visitor = trivial_visitor>
IRResult tree_augmentation_iterative_rounding(const Graph &g,
                                              EdgeSetOutputIterator solution,
                                              IRcomponents components =
                                                  IRcomponents(),
                                              Visitor visitor = Visitor()) {
    return tree_augmentation_iterative_rounding(
        g, boost::no_named_parameters(), std::move(solution),
        std::move(components), std::move(visitor));
}

} // ir
} // paal

#endif // PAAL_TREE_AUGMENTATION_HPP
