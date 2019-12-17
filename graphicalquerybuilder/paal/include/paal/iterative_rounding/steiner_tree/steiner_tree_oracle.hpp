//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_tree_oracle.hpp
 * @brief
 * @author Maciej Andrejczuk, Piotr Godlewski
 * @version 1.0
 * @date 2013-08-01
 */
#ifndef PAAL_STEINER_TREE_ORACLE_HPP
#define PAAL_STEINER_TREE_ORACLE_HPP

#include "paal/iterative_rounding/min_cut.hpp"
#include "paal/utils/irange.hpp"

#include <boost/optional.hpp>

#include <unordered_map>
#include <vector>

namespace paal {
namespace ir {

/**
 * @class steiner_tree_violation_checker
 * @brief Violations checker for the separation oracle
 *      in the steiner tree problem.
 */
class steiner_tree_violation_checker {
    using AuxEdge = min_cut_finder::Edge;
    using AuxVertex = min_cut_finder::Vertex;
    using AuxEdgeList = std::vector<AuxEdge>;
    using Violation = boost::optional<double>;

  public:
    using Candidate = AuxVertex;
    steiner_tree_violation_checker() : m_current_graph_size(-1) {}

    /**
     * Returns an iterator range of violated constraint candidates.
     */
    template <typename Problem, typename LP>
    auto get_violation_candidates(const Problem &problem, const LP &lp)
        ->decltype(irange(problem.get_terminals().size())) {

        int graph_size = problem.get_terminals().size();
        if (graph_size != m_current_graph_size) {
            // Graph has changed, construct new oracle
            m_current_graph_size = graph_size;
            m_root = select_root(problem.get_terminals());
            create_auxiliary_digraph(problem, lp);
        } else {
            update_auxiliary_digraph(problem, lp);
        }
        //TODO - rethink - why do we return the whole collection instead of returning m_root see check_violation implementation
        return irange(problem.get_terminals().size());
    }

    /**
     * Checks if the given constraint candidate is violated an if it is,
     * returns the violation value and violated constraint ID.
     */
    template <typename Problem>
    Violation check_violation(Candidate candidate, const Problem &problem) {
        if (candidate == m_root) {
            return Violation{};
        }

        double violation = find_violation(candidate);
        if (problem.get_compare().g(violation, 0)) {
            return violation;
        } else {
            return Violation{};
        }
    }

    /**
     * Adds the violated constraint to LP.
     * It contains all the components reachable from a given source,
     * but its sink vertex is not reachable.
     */
    template <typename Problem, typename LP>
    void add_violated_constraint(Candidate violating_terminal,
                                 const Problem &problem, LP &lp) {
        if (std::make_pair(violating_terminal, m_root) !=
            m_min_cut.get_last_cut()) {
            find_violation(violating_terminal);
        }

        auto const &components = problem.get_components();
        lp::linear_expression expr;
        for (int i = 0; i < components.size(); ++i) {
            auto u = m_artif_vertices[i];
            int ver = components.find_version(i);
            auto v = m_terminals_to_aux[problem.get_terminal_idx(components.find(i).get_sink(ver))];
            if (m_min_cut.is_in_source_set(u) &&
                !m_min_cut.is_in_source_set(v)) {
                expr += problem.find_column_lp(i);
            }
        }
        lp.add_row(std::move(expr) >= 1);
    }

  private:

    /**
     * @brief Creates the auxiliary directed graph used for feasibility testing
     *
     * Graph contains a vertex for each component and each terminal
     * Sources of every component have out edges with infinite weight
     * Target has in edge with weight x_i from LP
     */
    template <typename Problem, typename LP>
    void create_auxiliary_digraph(Problem &problem, const LP &lp) {
        m_min_cut.init(0);
        m_artif_vertices.clear();
        m_terminals_to_aux.clear();
        for (auto term : irange(problem.get_terminals().size())) {
            m_terminals_to_aux[term] = m_min_cut.add_vertex_to_graph();
        }
        auto const &components = problem.get_components();

        for (int i = 0; i < components.size(); ++i) {
            AuxVertex new_v = m_min_cut.add_vertex_to_graph();
            m_artif_vertices[i] = new_v;
            int ver = components.find_version(i);
            auto sink = components.find(i).get_sink(ver);
            for (auto w : boost::make_iterator_range(components.find(i)
                                                         .get_terminals())) {
                if (w != sink) {
                    double INF = std::numeric_limits<double>::max();
                    m_min_cut.add_edge_to_graph(m_terminals_to_aux[problem.get_terminal_idx(w)], new_v,
                                                INF);
                } else {
                    lp::col_id x = problem.find_column_lp(i);
                    double col_val = lp.get_col_value(x);
                    m_min_cut.add_edge_to_graph(new_v,
                            m_terminals_to_aux[problem.get_terminal_idx(sink)],
                            col_val);
                }
            }
        }
    }

    /**
     * Updates the auxiliary directed graph. Should be performed after each LP
     * iteration.
     */
    template <typename Problem, typename LP>
    void update_auxiliary_digraph(Problem &problem, const LP &lp) {
        auto const &components = problem.get_components();
        for (int i = 0; i < components.size(); ++i) {
            auto component_v = m_artif_vertices[i];
            int ver = components.find_version(i);
            auto sink = components.find(i).get_sink(ver);
            double col_val = lp.get_col_value(problem.find_column_lp(i));
            m_min_cut.set_capacity(component_v,
                m_terminals_to_aux[problem.get_terminal_idx(sink)], col_val);
        }
    }

    /**
     * Select the root terminal. Max-flow will be directed to that vertex during
     * LP oracle execution.
     */
    template <typename Terminals>
    AuxVertex select_root(const Terminals &terminals) {
        // TODO: Maybe it's better to select random vertex rather than first
        return 0;
    }

    /**
     * Finds the most violated cut containing \c src and returns its violation
     * value.
     */
    double find_violation(AuxVertex src) {
        double min_cut_weight = m_min_cut.find_min_cut(
            m_terminals_to_aux[src], m_terminals_to_aux[m_root]);
        return 1 - min_cut_weight;
    }

    AuxVertex m_root;         // root vertex, sink of all max-flows
    int m_current_graph_size; // size of current graph

    // maps component_id to aux_graph vertex
    std::unordered_map<int, AuxVertex> m_artif_vertices;

    // maps terminals to aux_graph vertices
    std::unordered_map<AuxVertex, AuxVertex> m_terminals_to_aux;

    min_cut_finder m_min_cut;
};

} //! ir
} //! paal
#endif // PAAL_STEINER_TREE_ORACLE_HPP
