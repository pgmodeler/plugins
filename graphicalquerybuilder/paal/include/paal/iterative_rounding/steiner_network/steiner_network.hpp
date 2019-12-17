//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2014 Piotr Godlewski
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_network.hpp
 * @brief
 * @author Piotr Wygocki, Piotr Godlewski
 * @version 1.0
 * @date 2013-06-24
 */
#ifndef PAAL_STEINER_NETWORK_HPP
#define PAAL_STEINER_NETWORK_HPP


#include "paal/iterative_rounding/ir_components.hpp"
#include "paal/iterative_rounding/iterative_rounding.hpp"
#include "paal/iterative_rounding/steiner_network/prune_restrictions_to_tree.hpp"
#include "paal/iterative_rounding/steiner_network/steiner_network_oracle.hpp"
#include "paal/lp/lp_row_generation.hpp"

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/range/as_array.hpp>

namespace paal {
namespace ir {

namespace {
struct steiner_network_compare_traits {
    static const double EPSILON;
};

const double steiner_network_compare_traits::EPSILON = 1e-10;
}


/**
 * @class steiner_network
 * @brief The class for solving the Steiner Network problem using Iterative
* Rounding.
 *
 * @tparam Graph input graph
 * @tparam Restrictions connectivity restrictions for vertex pairs
 * @tparam CostMap map of edge costs
 * @tparam ResultNetworkOutputIterator
 */
template <typename Graph, typename Restrictions, typename CostMap,
          typename VertexIndex, typename ResultNetworkOutputIterator,
          typename Oracle = paal::lp::random_violated_separation_oracle>
class steiner_network {
    using edge = typename boost::graph_traits<Graph>::edge_descriptor;
    using vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    using vertex_idx = decltype(get(std::declval<VertexIndex>(),
                                   std::declval<VertexIndex>()));

    using edge_map = std::unordered_map<lp::col_id, edge>;
    using edge_list = std::vector<edge>;
    using compare = utils::compare<double>;
    using error_message = boost::optional<std::string>;

    steiner_network_violation_checker m_violation_checker;
    const Graph &m_g;
    const Restrictions &m_restrictions;
    CostMap m_cost_map;
    VertexIndex m_index;
    ResultNetworkOutputIterator m_result_network;
    RestrictionsVector m_restrictions_vec;
    Oracle m_oracle;

    edge_map m_edge_map;
    edge_list m_result_list;
    compare m_compare;

public:

    /**
     * Constructor.
     */
    steiner_network(const Graph & g, const Restrictions & restrictions,
                    CostMap cost_map, VertexIndex vertex_index,
                    ResultNetworkOutputIterator result_network,
                    Oracle oracle = Oracle{}) :
            m_g(g), m_restrictions(restrictions),
            m_cost_map(cost_map), m_index(vertex_index), m_result_network(result_network),
            m_restrictions_vec(prune_restrictions_to_tree(m_restrictions, num_vertices(m_g))),
            m_oracle(oracle), m_compare(steiner_network_compare_traits::EPSILON) {}

    /**
     * Checks if the connectivity restrictions can be fulfilled.
     */
    error_message check_input_validity() {
        steiner_network_violation_checker checker;
        if (!checker.check_if_solution_exists(*this)) {
            return error_message{ "A Steiner network satisfying the "
                                 "restrictions does not exist." };
        } else {
            return error_message{};
        }
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
        using candidate = steiner_network_violation_checker::Candidate;
        return m_oracle([&](){return m_violation_checker.get_violation_candidates(*this, lp);},
                        [&](candidate c){return m_violation_checker.check_violation(c, *this);},
                        [&](candidate c){return m_violation_checker.add_violated_constraint(c, *this, lp);});
    }

    /**
     * Returns the map from LP column IDs to edges.
     */
    const edge_map &get_edge_map() const { return m_edge_map; }

    /**
     * Returns the input graph.
     */
    const Graph &get_graph() const { return m_g; }

    /**
     * Returns the vertex index.
     */
    const VertexIndex &get_index() const { return m_index; }

    /**
     * Returns the bigger of the two restrictions for a given vertex pair.
     */
    // TODO when automatic return is used g++-4.8.2 crashes
    auto get_max_restriction(vertex_idx u, vertex_idx v) const
        -> decltype(std::declval<Restrictions>()(0, 0))
    {
        return std::max(m_restrictions(u, v), m_restrictions(v, u));
    }

    /**
     * Returns the restrictions vector.
     */
    const RestrictionsVector &get_restrictions_vec() const
    {
        return m_restrictions_vec;
    }

    /**
     * Returns the cost of an edge.
     */
    // TODO when automatic return is used g++-4.8.2 crashes
    auto get_cost(edge e) -> decltype(get(std::declval<CostMap>(), e))
    {
        return get(m_cost_map, e);
    }

    /**
     * Binds a graph edge to a LP column.
     */
    void bind_edge_to_col(edge e, lp::col_id col) {
        m_edge_map.insert(typename edge_map::value_type(col, e));
    }

    /**
     * Removes an LP column and the graph edge corresponding to it.
     */
    void remove_column(lp::col_id col_id) {
        auto ret = m_edge_map.erase(col_id);
        assert(ret == 1);
    }

    /**
     * Adds an edge corresponding to the given column to the result set.
     */
    void add_column_to_solution(lp::col_id col_id) {
        auto e = col_to_edge(col_id);
        *m_result_network = e;
        ++m_result_network;
        m_result_list.push_back(e);
    }

    /**
     * Returns the list of edges that are already added to the solution.
     */
    const edge_list &get_edges_in_solution() const { return m_result_list; }

    /**
     * Returns the double comparison object.
     */
    compare get_compare() const { return m_compare; }

  private:

    edge col_to_edge(lp::col_id col) {
        auto i = m_edge_map.find(col);
        assert(i != m_edge_map.end());
        return i->second;
    }

};

namespace detail {
/**
 * @brief Creates a steiner_network object. Non-named version.
 *
 * @tparam Graph
 * @tparam Restrictions
 * @tparam CostMap
 * @tparam VertexIndex
 * @tparam ResultNetworkOutputIterator
 * @param g
 * @param restrictions
 * @param cost_map
 * @param vertex_index
 * @param result_network
 * @param oracle
 *
 * @return steiner_network object
 */
template <typename Oracle = lp::random_violated_separation_oracle,
    typename Graph, typename Restrictions, typename CostMap,
    typename VertexIndex, typename ResultNetworkOutputIterator>
steiner_network<Graph, Restrictions, CostMap, VertexIndex, ResultNetworkOutputIterator, Oracle>
make_steiner_network(const Graph & g, const Restrictions & restrictions,
                    CostMap cost_map, VertexIndex vertex_index,
                    ResultNetworkOutputIterator result_network,
                    Oracle oracle = Oracle{}) {
    return steiner_network<Graph, Restrictions, CostMap, VertexIndex,
                ResultNetworkOutputIterator, Oracle>(
                                    g, restrictions, cost_map, vertex_index, result_network, oracle);
}
} // detail

/**
 * Creates a steiner_network object. Named version.
 * The returned object can be used to check input validity or to get a lower
* bound on the
 * optimal solution cost.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam Restrictions
 * @tparam ResultNetworkOutputIterator
 * @tparam P
 * @tparam T
 * @tparam R
 * @param g
 * @param restrictions
 * @param params
 * @param result_network
 * @param oracle
 *
 * @return steiner_network object
 */
template <typename Oracle = lp::random_violated_separation_oracle,
    typename Graph, typename Restrictions, typename ResultNetworkOutputIterator,
    typename P, typename T, typename R>
auto
make_steiner_network(const Graph & g, const Restrictions & restrictions,
                    const boost::bgl_named_params<P, T, R>& params,
                    ResultNetworkOutputIterator result_network,
                    Oracle oracle = Oracle()) ->
       steiner_network<Graph, Restrictions,
            decltype(choose_const_pmap(get_param(params, boost::edge_weight), g, boost::edge_weight)),
            decltype(choose_const_pmap(get_param(params, boost::vertex_index), g, boost::vertex_index)),
            ResultNetworkOutputIterator, Oracle> {
    return detail::make_steiner_network(g, restrictions,
                choose_const_pmap(get_param(params, boost::edge_weight), g, boost::edge_weight),
                choose_const_pmap(get_param(params, boost::vertex_index), g, boost::vertex_index),
                result_network, oracle);
}

/**
 * Creates a steiner_network object. All default parameters.
 * The returned object can be used to check input validity or to get a lower
* bound on the
 * optimal solution cost.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam Restrictions
 * @tparam ResultNetworkOutputIterator
 * @param g
 * @param restrictions
 * @param result_network
 * @param oracle
 *
 * @return steiner_network object
 */
template <typename Oracle = lp::random_violated_separation_oracle,
    typename Graph, typename Restrictions, typename ResultNetworkOutputIterator>
auto
make_steiner_network(const Graph & g, const Restrictions & restrictions,
                    ResultNetworkOutputIterator result_network, Oracle oracle = Oracle()) ->
        decltype(make_steiner_network(g, restrictions, boost::no_named_parameters(), result_network, oracle)) {
    return make_steiner_network(g, restrictions, boost::no_named_parameters(), result_network, oracle);
}

/**
 * Initialization of the IR Steiner Network algorithm.
 */
struct steiner_network_init {
    /**
     * Initializes the LP: variables for edges.
     */
    template <typename Problem, typename LP>
    void operator()(Problem &problem, LP &lp) {
        lp.set_lp_name("steiner network");
        lp.set_optimization_type(lp::MINIMIZE);
        add_variables(problem, lp);
    }

  private:
    // adding variables
    template <typename Problem, typename LP>
    void add_variables(Problem &problem, LP &lp) {
        for (auto e : boost::as_array(edges(problem.get_graph()))) {
            lp::col_id col = lp.add_column(problem.get_cost(e), 0, 1);
            problem.bind_edge_to_col(e, col);
        }
    }
};

/**
 * Round Condition of the IR Steiner Network algorithm.
 */
struct steiner_network_round_condition {
    /**
     * Constructor. Takes epsilon used in double comparison.
     */
    steiner_network_round_condition(double epsilon =
                                        steiner_network_compare_traits::EPSILON)
        : m_round_half(epsilon), m_round_zero(epsilon) {}

    /**
     * Checks if a given column of the LP can be rounded to 0 or if it is
     * greater
     * then 1/2.
     * If the column is rounded to 0, the corresponding edge is removed from the
     * graph.
     * If the column is greater than 1/2, it is rounded to 1 and the
     * corresponding edge
     * is added to the solution.
     */
    template <typename Problem, typename LP>
    boost::optional<double> operator()(Problem & problem, const LP & lp, lp::col_id col_id) {
        auto ret = m_round_zero(problem, lp, col_id);
        if (ret) {
            //removing edge
            problem.remove_column(col_id);
            return ret;
        } else {
            ret = m_round_half(problem, lp, col_id);
            if (ret) {
                problem.add_column_to_solution(col_id);
                problem.remove_column(col_id);
            }
            return ret;
        }
    }

  private:
    round_condition_greater_than_half m_round_half;
    round_condition_equals<0> m_round_zero;
};

/**
 * Set Solution component of the IR Steiner Network algorithm.
 */
struct steiner_network_set_solution {
    /**
     * Constructor. Takes epsilon used in double comparison.
     */
    steiner_network_set_solution(double epsilon =
                                     steiner_network_compare_traits::EPSILON)
        : m_compare(epsilon) {}

    /**
     * Creates the result network form the LP (all edges corresponding to
     * columns with value 1).
     */
    template <typename Problem, typename GetSolution>
    void operator()(Problem &problem, const GetSolution &solution) {
        for (auto edge_and_col : problem.get_edge_map()) {
            if (m_compare.e(solution(edge_and_col.first), 1)) {
                problem.add_column_to_solution(edge_and_col.first);
            }
        }
    }

private:
    const utils::compare<double> m_compare;
};

template <typename Init = steiner_network_init,
          typename RoundCondition = steiner_network_round_condition,
          typename RelaxContition = utils::always_false,
          typename SetSolution = steiner_network_set_solution,
          typename SolveLPToExtremePoint = row_generation_solve_lp<>,
          typename ResolveLPToExtremePoint = row_generation_solve_lp<>>
using steiner_network_ir_components =
    IRcomponents<Init, RoundCondition, RelaxContition, SetSolution,
                 SolveLPToExtremePoint, ResolveLPToExtremePoint>;

namespace detail {
/**
 * @brief Solves the Steiner Network problem using Iterative Rounding. Non-named
* version.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam Restrictions
 * @tparam CostMap
 * @tparam VertexIndex
 * @tparam ResultNetworkOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @param g
 * @param restrictions
 * @param cost
 * @param vertex_index
 * @param result
 * @param components
 * @param oracle
 * @param visitor
 *
 * @return solution status
 */
template <typename Oracle = lp::random_violated_separation_oracle, typename Graph,
          typename Restrictions, typename CostMap, typename VertexIndex,
          typename ResultNetworkOutputIterator,
          typename IRcomponents = steiner_network_ir_components<>,
          typename Visitor = trivial_visitor>
IRResult steiner_network_iterative_rounding(
        const Graph & g,
        const Restrictions & restrictions,
        CostMap cost,
        VertexIndex vertex_index,
        ResultNetworkOutputIterator result,
        IRcomponents components = IRcomponents(),
        Oracle oracle = Oracle(),
        Visitor visitor = Visitor()) {
    auto steiner = make_steiner_network(g, restrictions, cost, vertex_index, result, oracle);
    return solve_iterative_rounding(steiner, std::move(components), std::move(visitor));
}
} // detail

/**
 * @brief Solves the Steiner Network problem using Iterative Rounding. Named
* version.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam Restrictions
 * @tparam ResultNetworkOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @tparam P
 * @tparam T
 * @tparam R
 * @param g
 * @param restrictions
 * @param params
 * @param result
 * @param components
 * @param oracle
 * @param visitor
 *
 * @return solution status
 */
template <typename Oracle = lp::random_violated_separation_oracle, typename Graph,
          typename Restrictions, typename ResultNetworkOutputIterator,
          typename IRcomponents = steiner_network_ir_components<>,
          typename Visitor = trivial_visitor, typename P, typename T,
          typename R>
IRResult steiner_network_iterative_rounding(
    const Graph &g, const Restrictions &restrictions,
    const boost::bgl_named_params<P, T, R> &params,
    ResultNetworkOutputIterator result,
    IRcomponents components = IRcomponents(), Oracle oracle = Oracle(),
    Visitor visitor = Visitor()) {
    return detail::steiner_network_iterative_rounding(
        g, restrictions,
        choose_const_pmap(get_param(params, boost::edge_weight), g,
                          boost::edge_weight),
        choose_const_pmap(get_param(params, boost::vertex_index), g,
                          boost::vertex_index),
        std::move(result), std::move(components), std::move(oracle),
        std::move(visitor));
}

/**
 * @brief Solves the Steiner Network problem using Iterative Rounding. All
* default parameters.
 *
 * @tparam Oracle
 * @tparam Graph
 * @tparam Restrictions
 * @tparam ResultNetworkOutputIterator
 * @tparam IRcomponents
 * @tparam Visitor
 * @param g
 * @param restrictions
 * @param result
 * @param components
 * @param oracle
 * @param visitor
 *
 * @return solution status
 */
template <typename Oracle = lp::random_violated_separation_oracle, typename Graph,
          typename Restrictions, typename ResultNetworkOutputIterator,
          typename IRcomponents = steiner_network_ir_components<>,
          typename Visitor = trivial_visitor>
IRResult steiner_network_iterative_rounding(const Graph &g,
                                            const Restrictions &restrictions,
                                            ResultNetworkOutputIterator result,
                                            IRcomponents components =
                                                IRcomponents(),
                                            Oracle oracle = Oracle(),
                                            Visitor visitor = Visitor()) {
    return steiner_network_iterative_rounding(
        g, restrictions, boost::no_named_parameters(), std::move(result),
        std::move(components), std::move(oracle), std::move(visitor));
}

} //! ir
} //! paal
#endif // PAAL_STEINER_NETWORK_HPP
