//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2014 Piotr Godlewski
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file iterative_rounding.hpp
 * @brief
 * @author Piotr Wygocki, Piotr Godlewski
 * @version 1.0
 * @date 2013-05-06
 */
#ifndef PAAL_ITERATIVE_ROUNDING_HPP
#define PAAL_ITERATIVE_ROUNDING_HPP


#include "paal/iterative_rounding/ir_components.hpp"
#include "paal/lp/glp.hpp"
#include "paal/utils/floating.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/irange.hpp"

#include <boost/optional.hpp>

#include <cstdlib>
#include <unordered_map>

namespace paal {
namespace ir {

/**
 * @brief Default Iterative Rounding visitor.
 */
struct trivial_visitor {
    /**
     * @brief Method called after (re)solving the LP.
     */
    template <typename Problem, typename LP>
    void solve_lp(Problem &problem, LP &lp) {}

    /**
     * @brief Method called after rounding a column of the LP.
     */
    template <typename Problem, typename LP>
    void round_col(Problem &problem, LP &lp, lp::col_id col, double val) {}

    /**
     * @brief Method called after relaxing a row of the LP.
     */
    template <typename Problem, typename LP>
    void relax_row(Problem &problem, LP &lp, lp::row_id row) {}
};

///default solve lp for row_generation,
///at first call PRIMAL, and DUAL on the next calls
template <typename Problem, typename LP>
class default_solve_lp_in_row_generation {
    bool m_first;
    LP & m_lp;
public:
    ///constructor
    default_solve_lp_in_row_generation(Problem &, LP & lp) : m_first(true), m_lp(lp) {}

    ///operator()
    lp::problem_type operator()()
    {
        if (m_first) {
            m_first = false;
            return m_lp.solve_simplex(lp::PRIMAL);
        }
        return m_lp.resolve_simplex(lp::DUAL);
    }
};

/// default row_generation for lp,
/// one can customize LP solving, by setting SolveLP
template <template <class, class> class SolveLP = default_solve_lp_in_row_generation>
struct row_generation_solve_lp {
///operator()
template <class Problem, class LP>
    auto operator()(Problem & problem, LP & lp) {
        return row_generation(problem.get_find_violation(lp), SolveLP<Problem, LP>(problem, lp));
    }
};



namespace detail {

    /**
     * @brief This class solves an iterative rounding problem.
 *
 * @tparam Problem
 * @tparam IRcomponents
 * @tparam Visitor
 * @tparam LP
 */
template <typename Problem, typename IRcomponents, typename Visitor = trivial_visitor, typename LP = lp::glp>
class iterative_rounding  {
    using RoundedCols = std::unordered_map<lp::col_id, std::pair<double, double>>;

    /**
     * @brief Returns the current value of the LP column.
     */
    double get_val(lp::col_id col) const {
        auto i = m_rounded.find(col);
        if (i == m_rounded.end()) {
            return m_lp.get_col_value(col);
        } else {
            return i->second.first;
        }
    }

  public:
    /**
     * @brief Constructor.
     */
    iterative_rounding(Problem &problem, IRcomponents e,
                       Visitor vis = Visitor())
        : m_ir_components(std::move(e)), m_visitor(std::move(vis)),
          m_problem(problem) {
        call<Init>(m_problem, m_lp);
    }

    /**
     * @brief Finds solution to the LP.
     *
     * @return LP solution status
     */
    lp::problem_type solve_lp() {
        auto prob_type = call<SolveLP>(m_problem, m_lp);
        assert(prob_type != lp::UNDEFINED);
        m_visitor.solve_lp(m_problem, m_lp);
        return prob_type;
    }

    /**
     * @brief Finds solution to the LP.
     *
     * @return LP solution status
     */
    lp::problem_type resolve_lp() {
        auto prob_type = call<ResolveLP>(m_problem, m_lp);
        assert(prob_type != lp::UNDEFINED);
        m_visitor.solve_lp(m_problem, m_lp);
        return prob_type;
    }

    /**
     * @brief Returns the solution cost based on the LP values.
     */
    double get_solution_cost() {
        double sol_cost(0);
        for (auto col : m_lp.get_columns()) {
            sol_cost += m_lp.get_col_value(col) * m_lp.get_col_coef(col);
        }
        for (auto rounded_col : m_rounded) {
            sol_cost += rounded_col.second.first * rounded_col.second.second;
        }
        return sol_cost;
    }

    /**
     * @brief Rounds the LP columns (independently) using the RoundCondition
     * component.
     *
     * @return true iff at least one column was rounded
     */
    bool round() {
        int deleted(0);
        auto &&cols = m_lp.get_columns();
        auto cbegin = std::begin(cols);
        auto cend = std::end(cols);

        while (cbegin != cend) {
            lp::col_id col = *cbegin;
            auto do_round = call<RoundCondition>(m_problem, m_lp, col);
            if (do_round) {
                ++deleted;
                m_rounded.insert(std::make_pair(col,
                    std::make_pair(*do_round, m_lp.get_col_coef(col))));
                m_visitor.round_col(m_problem, m_lp, col, *do_round);
                cbegin = delete_column(cbegin, *do_round);
            }
            else {
                ++cbegin;
            }
        }

        return deleted > 0;
    }

    /**
     * @brief Relaxes the LP rows using the RelaxCondition component.
     *
     * @return true iff at least one row was relaxed
     */
    bool relax() {
        int deleted(0);
        auto &&rows = m_lp.get_rows();
        auto rbegin = std::begin(rows);
        auto rend = std::end(rows);

        while (rbegin != rend) {
            lp::row_id row = *rbegin;
            if (call<RelaxCondition>(m_problem, m_lp, row)) {
                ++deleted;
                m_visitor.relax_row(m_problem, m_lp, row);
                rbegin = m_lp.delete_row(rbegin);
                if (call<RelaxationsLimit>(deleted)) {
                    break;
                }
            } else {
                ++rbegin;
            }
        }

        return deleted > 0;
    }

    /**
     * @brief Returns the LP object used to solve the IR.
     */
    LP &get_lp() { return m_lp; }

    /**
     * @brief Returns the IR components.
     */
    IRcomponents &get_ir_components() { return m_ir_components; }

    /**
     * @brief Sets the solution to the problem using SetSolution component.
     */
    void set_solution() {
        call<SetSolution>(m_problem, boost::bind(&iterative_rounding::get_val,
                                               this, _1));
    }

    /**
     * @brief Rounds the LP using the RoundCondition component.
     */
    void dependent_round() { call<RoundCondition>(m_problem, m_lp); }

    /**
     * @brief Checks if the IR problem has been solved, using the StopCondition
    * component.
     *
     * @return true iff the problem has been solved
     */
    bool stop_condition() { return call<StopCondition>(m_problem, m_lp); }

  private:
    template <typename Action, typename... Args>
    auto call(Args &&... args)
        ->decltype(std::declval<IRcomponents>().template call<Action>(
              std::forward<Args>(args)...)) {
        return m_ir_components.template call<Action>(
            std::forward<Args>(args)...);
    }

    /// Deletes a column from the LP and adjusts the row bounds.
    typename LP::ColIter
    delete_column(typename LP::ColIter col_iter, double value) {
        auto column = m_lp.get_rows_in_column(*col_iter);
        lp::row_id row;
        double coef;
        for (auto const &c : column) {
            boost::tie(row, coef) = c;
            double ub = m_lp.get_row_upper_bound(row);
            double lb = m_lp.get_row_lower_bound(row);
            double diff = coef * value;
            m_lp.set_row_upper_bound(row, ub - diff);
            m_lp.set_row_lower_bound(row, lb - diff);
        }
        return m_lp.delete_col(col_iter);
    };

    LP m_lp;
    IRcomponents m_ir_components;
    Visitor m_visitor;
    utils::compare<double> m_compare;
    RoundedCols m_rounded;
    Problem &m_problem;
};

} //detail

/// Iterative Rounding solution cost type. Solution cost only makes sense if the LP has been solved to optimal value.
using IRSolutionCost = boost::optional<double>;
/// Iterative Rounding result type: Pair consisting of LP problem type and IR
/// solution cost.
using IRResult = std::pair<lp::problem_type, IRSolutionCost>;

/**
 * @brief Solves an Iterative Rounding problem.
 *
 * @tparam Problem
 * @tparam IRcomponents
 * @tparam Visitor
 * @tparam LP
 * @param problem IR problem
 * @param components IR problem components
 * @param visitor visitor object used for logging progress of the algoithm
 */
template <typename Problem, typename IRcomponents, typename Visitor = trivial_visitor, typename LP = lp::glp>
IRResult solve_iterative_rounding(Problem & problem, IRcomponents components, Visitor visitor = Visitor()) {
    detail::iterative_rounding<Problem, IRcomponents, Visitor, LP> ir(problem, std::move(components), std::move(visitor));

    auto prob_type = ir.solve_lp();
    if (prob_type != lp::OPTIMAL) {
        return IRResult(prob_type, IRSolutionCost{});
    }

    while (!ir.stop_condition()) {
        bool rounded{ ir.round() };
        bool relaxed{ ir.relax() };
        assert(rounded || relaxed);

        prob_type = ir.resolve_lp();
        if (prob_type != lp::OPTIMAL) {
            return IRResult(prob_type, IRSolutionCost{});
        }
    }
    ir.set_solution();
    return IRResult(lp::OPTIMAL, IRSolutionCost(ir.get_solution_cost()));
}

/**
 * @brief Solves an Iterative Rounding problem with dependent rounding.
 *
 * @tparam Problem
 * @tparam IRcomponents
 * @tparam Visitor
 * @tparam LP
 * @param problem IR problem
 * @param components IR problem components
 * @param visitor visitor object used for logging progress of the algoithm
 */
template <typename Problem, typename IRcomponents, typename Visitor = trivial_visitor, typename LP = lp::glp>
IRResult solve_dependent_iterative_rounding(Problem & problem, IRcomponents components, Visitor visitor = Visitor()) {
    detail::iterative_rounding<Problem, IRcomponents, Visitor, LP> ir(problem, std::move(components), std::move(visitor));

    auto prob_type = ir.solve_lp();
    if (prob_type != lp::OPTIMAL) {
        return IRResult(prob_type, IRSolutionCost{});
    }

    while (!ir.stop_condition()) {
        ir.dependent_round();
        ir.relax();

        prob_type = ir.resolve_lp();
        if (prob_type != lp::OPTIMAL) {
            return IRResult(prob_type, IRSolutionCost{});
        }
    }
    ir.set_solution();
    return IRResult(lp::OPTIMAL, IRSolutionCost(ir.get_solution_cost()));
}

} // ir
} // paal

#endif // PAAL_ITERATIVE_ROUNDING_HPP
