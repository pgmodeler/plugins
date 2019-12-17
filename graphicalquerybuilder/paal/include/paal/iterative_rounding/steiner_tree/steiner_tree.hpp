//=======================================================================
// Copyright (c) 2013 Maciej Andrejczuk
//               2014 Piotr Godlewski, Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_tree.hpp
 * @brief
 * @author Maciej Andrejczuk, Piotr Godlewski, Piotr Wygocki
 * @version 1.0
 * @date 2013-08-01
 */
#ifndef PAAL_STEINER_TREE_HPP
#define PAAL_STEINER_TREE_HPP

#define BOOST_RESULT_OF_USE_DECLTYPE


#include "paal/data_structures/metric/basic_metrics.hpp"
#include "paal/iterative_rounding/ir_components.hpp"
#include "paal/iterative_rounding/iterative_rounding.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_components.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_strategy.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_tree_oracle.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_utils.hpp"
#include "paal/lp/lp_row_generation.hpp"
#include "paal/utils/assign_updates.hpp"

#include <boost/random/discrete_distribution.hpp>
#include <boost/range/join.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/find.hpp>

#include <random>
#include <vector>

namespace paal {
namespace ir {

namespace {
struct steiner_tree_compare_traits {
    static const double EPSILON;
};

const double steiner_tree_compare_traits::EPSILON = 1e-10;
}


/**
 * @class steiner_tree
 * @brief The class for solving the Steiner Tree problem using Iterative Rounding.
 *
 * @tparam OrigMetric
 * @tparam Terminals
 * @tparam Result
 * @tparam Strategy
 * @tparam Oracle separation oracle
 */
template<typename OrigMetric, typename Terminals, typename Result,
    typename Strategy = steiner_tree_all_generator,
    typename Oracle = lp::random_violated_separation_oracle>
class steiner_tree {
public:
    using MT = data_structures::metric_traits<OrigMetric>;
    using Vertex = typename MT::VertexType;
    using Vertices = std::vector<Vertex>;
    using Dist = typename MT::DistanceType;
    using Edge = typename std::pair<Vertex, Vertex>;
    using Compare = utils::compare<double>;
    using VertexIndex = data_structures::bimap<Vertex>;
    using MetricIdx = data_structures::array_metric<Dist>;
    using Metric = data_structures::metric_on_idx<
                        MetricIdx &,
                        const VertexIndex &,
                        data_structures::read_values_tag>;

private:
    Terminals m_terminals;        // terminals in current state
    Terminals m_steiner_vertices; // vertices that are not terminals
    VertexIndex m_terminals_index; // mapping terminals to numbers for 0 to n.
    VertexIndex m_vertex_index; // mapping vertices to numbers for 0 to n.
    MetricIdx m_cost_map_idx;            // metric in current state (operates on indexes)
    Metric m_cost_map;            // metric in current state
    steiner_components<Vertex, Dist> m_components; // components in current
                                                   // state
    Strategy m_strategy;      // strategy to generate the components
    Result m_result_iterator; // list of selected Steiner Vertices
    Vertices m_selected_elements; // list of selected Steiner Vertices
    Compare m_compare;        // comparison method

    std::unordered_map<int, lp::col_id> m_elements_map; // maps component_id ->
                                                        // col_id in LP
    steiner_tree_violation_checker m_violation_checker;

    Oracle m_oracle;

public:
    /**
     * Constructor.
     */
    steiner_tree(const OrigMetric& metric, const Terminals& terminals,
            const Terminals& steiner_vertices, Result result,
            const Strategy& strategy = Strategy{}, Oracle oracle = Oracle{}) :
        m_terminals(terminals), m_steiner_vertices(steiner_vertices),
        m_terminals_index(m_terminals),
        m_vertex_index(boost::range::join(m_terminals, m_steiner_vertices)),
        m_cost_map_idx(metric, boost::range::join(m_terminals, m_steiner_vertices)),
        m_cost_map(m_cost_map_idx, m_vertex_index),
        m_strategy(strategy), m_result_iterator(result),
        m_compare(steiner_tree_compare_traits::EPSILON), m_oracle(oracle) {
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
        using candidate = steiner_tree_violation_checker::Candidate;
        return m_oracle([&](){return m_violation_checker.get_violation_candidates(*this, lp);},
                        [&](candidate c){return m_violation_checker.check_violation(c, *this);},
                        [&](candidate c){return m_violation_checker.add_violated_constraint(c, *this, lp);});
    }

    /**
     * Generates all the components using specified strategy.
     */
    void gen_components() {
        m_strategy.gen_components(m_cost_map, m_terminals, m_steiner_vertices,
                                  m_components);
    }

    /**
     * Gets reference to all the components.
     */
    const steiner_components<Vertex, Dist> &get_components() const {
        return m_components;
    }

    /**
     * Gets reference to all the terminals.
     */
    const Terminals &get_terminals() const { return m_terminals; }

    /**
     * Returns the index of a terminal.
     */
    auto get_terminal_idx(Vertex v) const -> decltype(m_terminals_index.get_idx(v)) {
        return m_terminals_index.get_idx(v);
    }

    /**
     * Adds map entry from component id to LP lp::col_id.
     */
    void add_column_lp(int id, lp::col_id col) {
        bool b = m_elements_map.insert(std::make_pair(id, col)).second;
        assert(b);
    }

    /**
     * Finds LP lp::col_id based on component id.
     */
    lp::col_id find_column_lp(int id) const { return m_elements_map.at(id); }

    /**
     * Adds elements to solution.
     */
    void add_to_solution(const std::vector<Vertex>& steiner_elements) {
        boost::copy(steiner_elements, std::back_inserter(m_selected_elements));
    }

    /**
     * Removes duplicates and sets the final solution.
     */
    void set_solution() {
        boost::sort(m_selected_elements);
        boost::copy(boost::unique(m_selected_elements), m_result_iterator);
    }

    /**
     * Merges a component into its sink.
     */
    void update_graph(const steiner_component<Vertex, Dist>& selected) {
        auto const & all_terminals = selected.get_terminals();
        auto all_terminals_except_first = boost::make_iterator_range(++all_terminals.begin(), all_terminals.end());
        assert(!boost::empty(all_terminals));
        auto const & sink = all_terminals.front();
        for (auto t : all_terminals_except_first) {
            merge_vertices(sink, t);
            auto ii = boost::range::find(m_terminals, t);
            assert(ii != m_terminals.end());
            m_terminals.erase(ii);
        }
        // Clean components, they will be generated once again
        m_components.clear();
        m_elements_map.clear();
        m_terminals_index = VertexIndex(m_terminals);
    }

    /**
     * Gets comparison method.
     */
    utils::compare<double> get_compare() const {
        return m_compare;
    }

private:
    /**
     * Returns the index of a vertex.
     */
    auto get_idx(Vertex v) const -> decltype(m_vertex_index.get_idx(v)) {
        return m_vertex_index.get_idx(v);
    }

    /**
     * Recalculates distances after two vertices were merged.
     */
    void merge_vertices(Vertex u_vertex, Vertex w_vertex) {
        auto all_elements = boost::range::join(m_terminals, m_steiner_vertices);
        auto u = get_idx(u_vertex);
        auto w = get_idx(w_vertex);
        for (auto i_vertex: all_elements) {
            for (auto j_vertex: all_elements) {
                auto i = get_idx(i_vertex);
                auto j = get_idx(j_vertex);
                assign_min(m_cost_map_idx(i, j),
                           m_cost_map_idx(i, u) + m_cost_map_idx(w, j));
            }
        }
    }

};


/**
 * Initialization of the IR Steiner Tree algorithm.
 */
struct steiner_tree_init {
    /**
     * Initializes LP.
     */
    template <typename Problem, typename LP>
    void operator()(Problem &problem, LP &lp) {
        lp.clear();
        lp.set_lp_name("steiner tree");
        problem.gen_components();
        lp.set_optimization_type(lp::MINIMIZE);
        add_variables(problem, lp);
    }

  private:
    /**
     * Adds all the components as columns of LP.
     */
    template <typename Problem, typename LP>
    void add_variables(Problem &problem, LP &lp) {
        for (int i = 0; i < problem.get_components().size(); ++i) {
            lp::col_id col = lp.add_column(
                problem.get_components().find(i).get_cost(), 0, 1);
            problem.add_column_lp(i, col);
        }
    }
};

/**
 * Round Condition: step of iterative-randomized rounding algorithm.
 */
class steiner_tree_round_condition {
    std::default_random_engine m_rng;
  public:
    steiner_tree_round_condition(std::default_random_engine = std::default_random_engine{}) {}

    /**
     * Selects one component according to probability, adds it to solution and
     * merges selected vertices.
     */
    template <typename Problem, typename LP>
    void operator()(Problem &problem, LP &lp) {
        auto size = problem.get_components().size();
        std::vector<double> weights;
        weights.reserve(size);
        for (auto i : paal::irange(size)) {
            lp::col_id cId = problem.find_column_lp(i);
            weights.push_back(lp.get_col_value(cId));
        }

        auto selected = boost::random::discrete_distribution<>(weights)(m_rng);
        auto const &comp = problem.get_components().find(selected);
        problem.add_to_solution(comp.get_steiner_elements());
        problem.update_graph(comp);
        steiner_tree_init()(problem, lp);
    }
};

/**
 * Stop Condition: step of iterative-randomized rounding algorithm.
 */
struct steiner_tree_stop_condition {
    ///Checks if the IR algorithm should terminate.
    template<typename Problem, typename LP>
    bool operator()(Problem& problem, LP &) {
        return problem.get_terminals().size() < 2;
    }
};

/**
 * Set Solution component.
 */
struct steiner_tree_set_solution {
    /**
     * Removes duplicates from selected Steiner vertices list.
     */
    template <typename Problem, typename GetSolution>
    void operator()(Problem & problem, const GetSolution &) {
        problem.set_solution();
    }
};

/**
 * Makes steiner_tree object. Just to avoid providing type names in template.
 */
template<typename Oracle = lp::random_violated_separation_oracle,
        typename OrigMetric, typename Terminals, typename Result, typename Strategy>
steiner_tree<OrigMetric, Terminals, Result, Strategy, Oracle> make_steiner_tree(
        const OrigMetric& metric, const Terminals& terminals,
        const Terminals& steiner_vertices, Result result, const Strategy& strategy,
        Oracle oracle = Oracle()) {
    return steiner_tree<OrigMetric, Terminals, Result, Strategy, Oracle>(metric,
            terminals, steiner_vertices, result, strategy, oracle);
}

template <typename Init = steiner_tree_init,
          typename RoundCondition = steiner_tree_round_condition,
          typename RelaxCondition = utils::always_false,
          typename SetSolution = steiner_tree_set_solution,
          typename SolveLPToExtremePoint = row_generation_solve_lp<>,
          typename ResolveLPToExtremePoint = row_generation_solve_lp<>,
          typename StopCondition = steiner_tree_stop_condition>
using steiner_tree_ir_components =
    IRcomponents<Init, RoundCondition, RelaxCondition, SetSolution,
                 SolveLPToExtremePoint, ResolveLPToExtremePoint, StopCondition>;

/**
 * @brief Solves the Steiner Tree problem using Iterative Rounding.
 */
template <typename Oracle = lp::random_violated_separation_oracle,
          typename Strategy = steiner_tree_all_generator,
          typename OrigMetric,
          typename Terminals,
          typename Result,
          typename IRcomponents = steiner_tree_ir_components<>,
          typename Visitor = trivial_visitor>
lp::problem_type steiner_tree_iterative_rounding(const OrigMetric& metric, const Terminals& terminals,
        const Terminals& steiner_vertices, Result result, Strategy strategy = Strategy{},
        IRcomponents comps = IRcomponents{}, Oracle oracle = Oracle{},
        Visitor visitor = Visitor{}) {

    auto steiner = paal::ir::make_steiner_tree(metric, terminals, steiner_vertices, result, strategy, oracle);
    auto res = paal::ir::solve_dependent_iterative_rounding(steiner, std::move(comps), std::move(visitor));
    return res.first;
}

} //! ir
} //! paal
#endif // PAAL_STEINER_TREE_HPP
