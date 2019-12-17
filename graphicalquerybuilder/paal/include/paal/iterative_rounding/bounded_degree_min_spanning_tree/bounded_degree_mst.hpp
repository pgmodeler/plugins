//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file bounded_degree_mst.hpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2013-06-03
 */
#ifndef PAAL_BOUNDED_DEGREE_MST_HPP
#define PAAL_BOUNDED_DEGREE_MST_HPP


#include "paal/iterative_rounding/bounded_degree_min_spanning_tree/bounded_degree_mst_oracle.hpp"
#include "paal/iterative_rounding/ir_components.hpp"
#include "paal/iterative_rounding/iterative_rounding.hpp"
#include "paal/lp/lp_row_generation.hpp"

#include <boost/bimap.hpp>
#include <boost/range/as_array.hpp>
#include <boost/graph/connected_components.hpp>

namespace paal {
namespace ir {

namespace {
struct bounded_degree_mst_compare_traits {
    static const double EPSILON;
};

const double bounded_degree_mst_compare_traits::EPSILON = 1e-10;
}

/**
 * @class bounded_degree_mst
 * @brief The class for solving the Bounded Degree MST problem using Iterative
* Rounding.
 *
 * @tparam Graph input graph
 * @tparam DegreeBounds map from Graph vertices to degree bounds
 * @tparam CostMap map from Graph edges to costs
 * @tparam VertexIndex map from Graph vertices to indices
 * @tparam SpanningTreeOutputIterator
 * @tparam Oracle separation oracle
 */
template <typename Graph, typename DegreeBounds, typename CostMap,
          typename VertexIndex, typename SpanningTreeOutputIterator,
          typename Oracle = paal::lp::random_violated_separation_oracle>
class bounded_degree_mst {
  public:
    /**
     * Constructor.
     */
    bounded_degree_mst(const Graph & g, const DegreeBounds & deg_bounds,
                    CostMap cost_map, VertexIndex index,
                    SpanningTreeOutputIterator result_spanning_tree, Oracle oracle = Oracle{}) :
              m_g(g), m_cost_map(cost_map), m_index(index), m_deg_bounds(deg_bounds),
              m_result_spanning_tree(result_spanning_tree),
              m_compare(bounded_degree_mst_compare_traits::EPSILON),
              m_oracle(oracle)
    {}

    using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;

    using EdgeMap = boost::bimap<Edge, lp::col_id>;
    using VertexMap = std::unordered_map<lp::row_id, Vertex>;

    using EdgeMapOriginal = std::vector<std::pair<lp::col_id, Edge>>;

    using ErrorMessage = boost::optional<std::string>;

    /**
     * Checks if the input graph is connected.
     */
    ErrorMessage check_input_validity() {
        // Is g connected?
        std::vector<int> components(num_vertices(m_g));
        int num = boost::connected_components(m_g, &components[0]);

        if (num > 1) {
            return ErrorMessage{ "The graph is not connected." };
        }

        return ErrorMessage{};
    }

    /**
     * @brief
     *
     * @tparam LP
     * @param lp
     *
     * @return
     */
    template <typename LP>
    auto get_find_violation(LP & lp) {
        using candidate = bdmst_violation_checker::Candidate;
        return m_oracle([&](){return m_violation_checker.get_violation_candidates(*this, lp);},
                        [&](candidate c){return m_violation_checker.check_violation(c, *this);},
                        [&](candidate c){return m_violation_checker.add_violated_constraint(c, *this, lp);});
    }

    /**
     * Returns the input graph.
     */
    const Graph &get_graph() const { return m_g; }

    /**
     * Returns the vertex index.
     */
    const VertexIndex &get_index() const { return m_index; }

    /**
     * Removes an LP column and the graph edge corresponding to it.
     */
    void remove_column(lp::col_id col_id) {
        auto ret = m_edge_map.right.erase(col_id);
        assert(ret == 1);
    }

    /**
     * Binds a graph edge to a LP column.
     */
    void bind_edge_to_col(Edge e, lp::col_id col) {
        m_edge_map_original.push_back(
            typename EdgeMapOriginal::value_type(col, e));
        m_edge_map.insert(typename EdgeMap::value_type(e, col));
    }

    /**
     * Returns the cost of a given edge.
     */
    decltype(get(std::declval<CostMap>(), std::declval<Edge>()))
        get_cost(Edge e) {
        return get(m_cost_map, e);
    }

    /**
     * Returns the degree bound of a vertex.
     */
    decltype(std::declval<DegreeBounds>()(get(std::declval<VertexIndex>(),
                                              std::declval<Vertex>())))
        get_degree_bound(Vertex v) {
        return m_deg_bounds(get(m_index, v));
    }

    /**
     * Returns the LP column corresponding to an edge, if it wasn't deleted from
     * the LP.
     */
    boost::optional<lp::col_id> edge_to_col(Edge e) const {
        auto i = m_edge_map.left.find(e);
        if (i != m_edge_map.left.end()) {
            return i->second;
        } else {
            return boost::none;
        }
    }

    /**
     * Returns a bimap between edges and LP column IDs.
     */
    const EdgeMap &get_edge_map() const { return m_edge_map; }

    /**
     * Returns a mapping between LP column IDs and edges in the original graph.
     */
    const EdgeMapOriginal &get_original_edges_map() const {
        return m_edge_map_original;
    }

    /**
     * Adds an edge to the result spanning tree.
     */
    void add_to_result_spanning_tree(Edge e) {
        *m_result_spanning_tree = e;
        ++m_result_spanning_tree;
    }

    /**
     * Returns the double comparison object.
     */
    utils::compare<double> get_compare() const {
        return m_compare;
    }

    /**
     * Binds a graph vertex to an LP row.
     */
    void bind_vertex_to_row(Vertex v, lp::row_id row) {
        m_vertex_map.insert(typename VertexMap::value_type(row, v));
    }

    /**
     * Unbinds the graph vertex from its corresponding (deleted) LP row.
     */
    void remove_row(lp::row_id row_id) {
        auto ret = m_vertex_map.erase(row_id);
        assert(ret == 1);
    }

    /**
     * Returns the graph vertex corresponding to a given LP row,
     *        unless the row doen't correspond to any vertex.
     */
    boost::optional<Vertex> row_to_vertex(lp::row_id row) {
        auto i = m_vertex_map.find(row);
        if (i != m_vertex_map.end()) {
            return i->second;
        } else {
            return boost::none;
        }
    }

  private:
    Edge col_to_edge(lp::col_id col) {
        auto i = m_edge_map.right.find(col);
        assert(i != m_edge_map.right.end());
        return i->second;
    }

    const Graph &m_g;
    CostMap m_cost_map;
    VertexIndex m_index;
    const DegreeBounds &m_deg_bounds;
    SpanningTreeOutputIterator m_result_spanning_tree;
    bdmst_violation_checker m_violation_checker;

    EdgeMapOriginal m_edge_map_original;
    EdgeMap m_edge_map;
    VertexMap m_vertex_map;

    const utils::compare<double>   m_compare;

    Oracle m_oracle;
};

namespace detail {
/**
 * @brief Creates a bounded_degree_mst object. Non-named version.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam DegreeBounds
 * @tparam CostMap
 * @tparam VertexIndex
 * @tparam SpanningTreeOutputIterator
 * @param g
 * @param degBoundMap
 * @param cost_map
 * @param vertex_index
 * @param result_spanning_tree
 * @param oracle
 *
 * @return bounded_degree_mst object
 */
template <typename Oracle = lp::random_violated_separation_oracle,
          typename Graph,
          typename DegreeBounds, typename CostMap, typename VertexIndex,
          typename SpanningTreeOutputIterator>
bounded_degree_mst<Graph, DegreeBounds, CostMap, VertexIndex, SpanningTreeOutputIterator, Oracle>
make_bounded_degree_mst(const Graph & g, const DegreeBounds & deg_bounds,
                      CostMap cost_map, VertexIndex vertex_index,
                      SpanningTreeOutputIterator result_spanning_tree,
                      Oracle oracle = Oracle()) {
    return bounded_degree_mst<Graph, DegreeBounds, CostMap, VertexIndex,
                SpanningTreeOutputIterator, Oracle>(g, deg_bounds, cost_map, vertex_index,
                    result_spanning_tree, oracle);
}
} // detail

/**
 * Creates a bounded_degree_mst object. Named version.
 * The returned object can be used to check input validity or to get a lower
* bound on the
 * optimal solution cost.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam DegreeBounds
 * @tparam SpanningTreeOutputIterator
 * @tparam P
 * @tparam T
 * @tparam R
 * @param g
 * @param deg_bounds
 * @param params
 * @param result_spanning_tree
 * @param oracle
 *
 * @return bounded_degree_mst object
 */
template <typename Oracle = lp::random_violated_separation_oracle,
          typename Graph,
          typename DegreeBounds, typename SpanningTreeOutputIterator,
          typename P, typename T, typename R>
auto
make_bounded_degree_mst(const Graph & g,
                      const DegreeBounds & deg_bounds,
                      const boost::bgl_named_params<P, T, R> & params,
                      SpanningTreeOutputIterator result_spanning_tree,
                      Oracle oracle = Oracle())
        -> bounded_degree_mst<Graph, DegreeBounds,
                decltype(choose_const_pmap(get_param(params, boost::edge_weight), g, boost::edge_weight)),
                decltype(choose_const_pmap(get_param(params, boost::vertex_index), g, boost::vertex_index)),
                SpanningTreeOutputIterator, Oracle> {

    return detail::make_bounded_degree_mst(g, deg_bounds,
                choose_const_pmap(get_param(params, boost::edge_weight), g, boost::edge_weight),
                choose_const_pmap(get_param(params, boost::vertex_index), g, boost::vertex_index),
                result_spanning_tree, oracle);
}

/**
 * Creates a bounded_degree_mst object. All default parameters.
 * The returned object can be used to check input validity or to get a lower
* bound on the
 * optimal solution cost.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam DegreeBounds
 * @tparam SpanningTreeOutputIterator
 * @param g
 * @param deg_bounds
 * @param result_spanning_tree
 * @param oracle
 *
 * @return bounded_degree_mst object
 */
template <typename Oracle = lp::random_violated_separation_oracle,
          typename Graph, typename DegreeBounds, typename SpanningTreeOutputIterator>
auto
make_bounded_degree_mst(const Graph & g, const DegreeBounds & deg_bounds,
                      SpanningTreeOutputIterator result_spanning_tree,
                      Oracle oracle = Oracle()) ->
        decltype(make_bounded_degree_mst(g, deg_bounds, boost::no_named_parameters(), result_spanning_tree, oracle)) {
    return make_bounded_degree_mst(g, deg_bounds, boost::no_named_parameters(), result_spanning_tree, oracle);
}

/**
 * Round Condition of the IR Bounded Degree MST algorithm.
 */
struct bdmst_round_condition {
    /**
     * Constructor. Takes epsilon used in double comparison.
     */
    bdmst_round_condition(double epsilon =
                              bounded_degree_mst_compare_traits::EPSILON)
        : m_round_zero(epsilon) {}

    /**
     * Checks if a given column of the LP can be rounded to 0.
     * If the column is rounded, the corresponding edge is removed from the
     * graph.
     */
    template <typename Problem, typename LP>
    boost::optional<double> operator()(Problem &problem, const LP &lp,
                                       lp::col_id col) {
        auto ret = m_round_zero(problem, lp, col);
        if (ret) {
            problem.remove_column(col);
        }
        return ret;
    }

  private:
    round_condition_equals<0> m_round_zero;
};

/**
 * Relax Condition of the IR Bounded Degree MST algorithm.
 */
struct bdmst_relax_condition {
    /**
     * Checks if a given row of the LP corresponds to a degree bound and can be
     * relaxed.
     * If the row degree is not greater than the corresponding degree bound + 1,
     * it is relaxed
     * and the degree bound is deleted from the problem.
     */
    template <typename Problem, typename LP>
    bool operator()(Problem &problem, const LP &lp, lp::row_id row) {
        auto vertex = problem.row_to_vertex(row);
        if (vertex) {
            auto ret = (lp.get_row_degree(row) <=
                        problem.get_degree_bound(*vertex) + 1);
            if (ret) {
                problem.remove_row(row);
            }
            return ret;
        } else
            return false;
    }
};

/**
 * Initialization of the IR Bounded Degree MST algorithm.
 */
struct bdmst_init {
    /**
     * Initializes the LP: variables for edges, degree bound constraints
     * and constraint for all edges.
     */
    template <typename Problem, typename LP>
    void operator()(Problem &problem, LP &lp) {
        lp.set_lp_name("bounded degree minimum spanning tree");
        lp.set_optimization_type(lp::MINIMIZE);

        add_variables(problem, lp);
        add_degree_bound_constraints(problem, lp);
        add_all_set_equality(problem, lp);
    }

  private:
    /**
     * Adds a variable to the LP for each edge in the input graph.
     * Binds the LP columns to edges.
     */
    template <typename Problem, typename LP>
    void add_variables(Problem & problem, LP & lp) {
        for (auto e : boost::as_array(edges(problem.get_graph()))) {
            auto col = lp.add_column(problem.get_cost(e), 0, 1);
            problem.bind_edge_to_col(e, col);
        }
    }

    /**
     * Adds a degree bound constraint to the LP for each vertex in the input
     * graph
     * and binds vertices to rows.
     */
    template <typename Problem, typename LP>
    void add_degree_bound_constraints(Problem &problem, LP &lp) {
        auto const &g = problem.get_graph();

        for (auto v : boost::as_array(vertices(g))) {
            lp::linear_expression expr;

            for (auto e : boost::as_array(out_edges(v, g))) {
                expr += *(problem.edge_to_col(e));
            }

            auto row =
                lp.add_row(std::move(expr) <= problem.get_degree_bound(v));
            problem.bind_vertex_to_row(v, row);
        }
    }

    /**
     * Adds an equality constraint to the LP for the set of all edges in the
     * input graph.
     */
    template <typename Problem, typename LP>
    void add_all_set_equality(Problem &problem, LP &lp) {
        lp::linear_expression expr;
        for (auto col : lp.get_columns()) {
            expr += col;
        }
        lp.add_row(std::move(expr) == num_vertices(problem.get_graph()) - 1);
    }
};

/**
 * Set Solution component of the IR Bounded Degree MST algorithm.
 */
struct bdmst_set_solution {
    /**
     * Constructor. Takes epsilon used in double comparison.
     */
    bdmst_set_solution(double epsilon =
                           bounded_degree_mst_compare_traits::EPSILON)
        : m_compare(epsilon) {}

    /**
     * Creates the result spanning tree form the LP (all edges corresponding to
     * columns with value 1).
     */
    template <typename Problem, typename GetSolution>
    void operator()(Problem & problem, const GetSolution & solution) {
        for (auto col_and_edge : problem.get_original_edges_map()) {
            if (m_compare.e(solution(col_and_edge.first), 1)) {
                problem.add_to_result_spanning_tree(col_and_edge.second);
            }
        }
    }

private:
    const utils::compare<double>   m_compare;
};

template <typename Init = bdmst_init,
          typename RoundCondition = bdmst_round_condition,
          typename RelaxContition = bdmst_relax_condition,
          typename SetSolution = bdmst_set_solution,
          typename SolveLPToExtremePoint = ir::row_generation_solve_lp<>,
          typename ResolveLpToExtremePoint = ir::row_generation_solve_lp<>>
using bdmst_ir_components =
    IRcomponents<Init, RoundCondition, RelaxContition, SetSolution,
                 SolveLPToExtremePoint, ResolveLpToExtremePoint>;

namespace detail {
/**
 * @brief Solves the Bounded Degree MST problem using Iterative Rounding.
* Non-named version.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam DegreeBounds
 * @tparam CostMap
 * @tparam VertexIndex
 * @tparam SpanningTreeOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @param g
 * @param degBoundMap
 * @param cost_map
 * @param vertex_index
 * @param result_spanning_tree
 * @param components
 * @param oracle
 * @param visitor
 *
 * @return solution status
 */
template <typename Oracle = lp::random_violated_separation_oracle,
          typename Graph,
          typename DegreeBounds, typename CostMap, typename VertexIndex,
          typename SpanningTreeOutputIterator,
          typename IRcomponents = bdmst_ir_components<>,
          typename Visitor = trivial_visitor>
IRResult bounded_degree_mst_iterative_rounding(
        const Graph & g,
        const DegreeBounds & deg_bounds,
        CostMap cost_map,
        VertexIndex vertex_index,
        SpanningTreeOutputIterator result_spanning_tree,
        IRcomponents components = IRcomponents(),
        Oracle oracle = Oracle(),
        Visitor visitor = Visitor()) {

    auto bdmst = make_bounded_degree_mst(g, deg_bounds, cost_map, vertex_index, result_spanning_tree, oracle);
    return solve_iterative_rounding(bdmst, std::move(components), std::move(visitor));
}
} // detail

/**
 * @brief Solves the Bounded Degree MST problem using Iterative Rounding. Named
* version.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam DegreeBounds
 * @tparam SpanningTreeOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @tparam P
 * @tparam T
 * @tparam R
 * @param g
 * @param deg_bounds
 * @param result_spanning_tree
 * @param params
 * @param components
 * @param oracle
 * @param visitor
 *
 * @return solution status
 */
template <typename Oracle = lp::random_violated_separation_oracle,
          typename Graph,
          typename DegreeBounds, typename SpanningTreeOutputIterator,
          typename IRcomponents = bdmst_ir_components<>,
          typename Visitor = trivial_visitor, typename P, typename T,
          typename R>
IRResult bounded_degree_mst_iterative_rounding(
            const Graph & g,
            const DegreeBounds & deg_bounds,
            const boost::bgl_named_params<P, T, R> & params,
            SpanningTreeOutputIterator result_spanning_tree,
            IRcomponents components = IRcomponents(),
            Oracle oracle = Oracle(),
            Visitor visitor = Visitor()) {

        return detail::bounded_degree_mst_iterative_rounding(g, deg_bounds,
                    choose_const_pmap(get_param(params, boost::edge_weight), g, boost::edge_weight),
                    choose_const_pmap(get_param(params, boost::vertex_index), g, boost::vertex_index),
                    std::move(result_spanning_tree), std::move(components),
                    std::move(oracle), std::move(visitor));
}

/**
 * @brief Solves the Bounded Degree MST problem using Iterative Rounding. All
* default parameters.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam DegreeBounds
 * @tparam SpanningTreeOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @param g
 * @param deg_bounds
 * @param result_spanning_tree
 * @param components
 * @param oracle
 * @param visitor
 *
 * @return solution status
 */
template <typename Oracle = lp::random_violated_separation_oracle, typename Graph,
          typename DegreeBounds, typename SpanningTreeOutputIterator,
          typename IRcomponents = bdmst_ir_components<>,
          typename Visitor = trivial_visitor>
IRResult bounded_degree_mst_iterative_rounding(
            const Graph & g,
            const DegreeBounds & deg_bounds,
            SpanningTreeOutputIterator result_spanning_tree,
            IRcomponents components = IRcomponents(),
            Oracle oracle = Oracle(),
            Visitor visitor = Visitor()) {

        return bounded_degree_mst_iterative_rounding(g, deg_bounds,
                    boost::no_named_parameters(), std::move(result_spanning_tree),
                    std::move(components), std::move(oracle), std::move(visitor));
}

} //! ir
} //! paal
#endif // PAAL_BOUNDED_DEGREE_MST_HPP
