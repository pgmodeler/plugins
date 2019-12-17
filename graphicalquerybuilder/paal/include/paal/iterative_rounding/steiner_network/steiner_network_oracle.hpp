//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2014 Piotr Godlewski
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_network_oracle.hpp
 * @brief
 * @author Piotr Wygocki, Piotr Godlewski
 * @version 1.0
 * @date 2013-06-24
 */
#ifndef PAAL_STEINER_NETWORK_ORACLE_HPP
#define PAAL_STEINER_NETWORK_ORACLE_HPP

#include "paal/iterative_rounding/min_cut.hpp"

#include <boost/range/as_array.hpp>

namespace paal {
namespace ir {

/**
 * @class steiner_network_violation_checker
 * @brief Violations checker for the separation oracle
 *      in the steiner network problem.
 */
class steiner_network_violation_checker {
    using AuxVertex = min_cut_finder::Vertex;
    using Violation = double;

  public:
    using Candidate = std::pair<AuxVertex, AuxVertex>;

    /**
     * Checks if any solution to the problem exists.
     */
    template <typename Problem>
    bool check_if_solution_exists(Problem &problem) {
        auto const &g = problem.get_graph();
        auto const &index = problem.get_index();
        m_min_cut.init(num_vertices(g));

        for (auto e : boost::as_array(edges(g))) {
            auto u = get(index, source(e, g));
            auto v = get(index, target(e, g));
            m_min_cut.add_edge_to_graph(u, v, 1, 1);
        }

        for (auto res : problem.get_restrictions_vec()) {
            if (check_violation(res, problem)) {
                return false;
            }
        }

        return true;
    }

    /**
     * Returns an iterator range of violated constraint candidates.
     */
    template <typename Problem, typename LP>
    auto get_violation_candidates(const Problem &problem, const LP &lp)
        ->decltype(problem.get_restrictions_vec()) {

        fill_auxiliary_digraph(problem, lp);
        return problem.get_restrictions_vec();
    }

    /**
     * Checks if the given constraint candidate is violated an if it is,
     * returns the violation value and violated constraint ID.
     */
    template <typename Problem>
    Violation check_violation(Candidate candidate, const Problem &problem) {
        double violation =
            find_violation(candidate.first, candidate.second, problem);
        if (problem.get_compare().g(violation, 0)) {
            return violation;
        } else {
            return Violation{};
        }
    }

    /**
     * Adds a violated constraint to the LP.
     */
    template <typename Problem, typename LP>
    void add_violated_constraint(Candidate violation, const Problem &problem,
                                 LP &lp) {
        if (violation != m_min_cut.get_last_cut()) {
            find_violation(violation.first, violation.second, problem);
        }

        auto const &g = problem.get_graph();
        auto const &index = problem.get_index();
        auto restriction =
            problem.get_max_restriction(violation.first, violation.second);

        for (auto const &e : problem.get_edges_in_solution()) {
            if (is_edge_in_violating_cut(e, g, index)) {
                --restriction;
            }
        }

        lp::linear_expression expr;
        for (auto const &e : problem.get_edge_map()) {
            if (is_edge_in_violating_cut(e.second, g, index)) {
                expr += e.first;
            }
        }

        lp.add_row(std::move(expr) >= restriction);
    }

  private:

    /**
     * Checks if a given edge belongs to the cut given by the current violating
     * set.
     */
    template <typename Edge, typename Graph, typename Index>
    bool is_edge_in_violating_cut(Edge edge, const Graph &g,
                                  const Index &index) {
        auto u = get(index, source(edge, g));
        auto v = get(index, target(edge, g));
        return m_min_cut.is_in_source_set(u) != m_min_cut.is_in_source_set(v);
    }

    /**
     * Creates the auxiliary directed graph used for feasibility testing.
     */
    template <typename Problem, typename LP>
    void fill_auxiliary_digraph(Problem &problem, const LP &lp) {
        auto const &g = problem.get_graph();
        auto const &index = problem.get_index();
        m_min_cut.init(num_vertices(g));

        for (auto const &e : problem.get_edge_map()) {
            lp::col_id col_idx = e.first;
            double col_val = lp.get_col_value(col_idx);

            if (problem.get_compare().g(col_val, 0)) {
                auto u = get(index, source(e.second, g));
                auto v = get(index, target(e.second, g));
                m_min_cut.add_edge_to_graph(u, v, col_val, col_val);
            }
        }

        for (auto const &e : problem.get_edges_in_solution()) {
            auto u = get(index, source(e, g));
            auto v = get(index, target(e, g));
            m_min_cut.add_edge_to_graph(u, v, 1, 1);
        }
    }

    /**
     * Finds the most violated set of vertices containing \c src and not
     * containing \c trg and returns its violation value.
     * @param src vertex to be contained in the violating set
     * @param trg vertex not to be contained in the violating set
     * @param problem problem object
     * @return violation of the found set
     */
    template <typename Problem>
    double find_violation(AuxVertex src, AuxVertex trg,
                          const Problem &problem) {
        double min_cut_weight = m_min_cut.find_min_cut(src, trg);
        double restriction = problem.get_max_restriction(src, trg);
        return restriction - min_cut_weight;
    }

    min_cut_finder m_min_cut;
};

} //! ir
} //! paal
#endif // PAAL_STEINER_NETWORK_ORACLE_HPP
