//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file bounded_degree_mst_oracle.hpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2013-06-05
 */
#ifndef PAAL_BOUNDED_DEGREE_MST_ORACLE_HPP
#define PAAL_BOUNDED_DEGREE_MST_ORACLE_HPP

#include "paal/iterative_rounding/min_cut.hpp"
#include "paal/lp/lp_base.hpp"

#include <boost/optional.hpp>
#include <boost/range/as_array.hpp>

#include <vector>

namespace paal {
namespace ir {

/**
 * @class bdmst_violation_checker
 * @brief Violations checker for the separation oracle
 *      in the bounded degree minimum spanning tree problem.
 */
class bdmst_violation_checker {
    using AuxEdge = min_cut_finder::Edge;
    using AuxVertex = min_cut_finder::Vertex;
    using AuxEdgeList = std::vector<AuxEdge>;
    using Violation = boost::optional<double>;

  public:
    using Candidate = std::pair<AuxVertex, AuxVertex>;
    using CandidateList = std::vector<Candidate>;

    /**
     * Returns an iterator range of violated constraint candidates.
     */
    template <typename Problem, typename LP>
    const CandidateList &get_violation_candidates(const Problem &problem,
                                                  const LP &lp) {
        fill_auxiliary_digraph(problem, lp);
        initialize_candidates(problem);
        return m_candidate_list;
    }

    /**
     * Checks if the given constraint candidate is violated an if it is,
     * returns the violation value and violated constraint ID.
     */
    template <typename Problem>
    Violation check_violation(Candidate candidate, const Problem &problem) {
        double violation = find_violation(candidate.first, candidate.second);
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
    void add_violated_constraint(Candidate violating_pair,
                                 const Problem &problem, LP &lp) {
        if (violating_pair != m_min_cut.get_last_cut()) {
            find_violation(violating_pair.first, violating_pair.second);
        }

        auto const &g = problem.get_graph();
        auto const &index = problem.get_index();

        lp::linear_expression expr;
        for (auto const &e : problem.get_edge_map().right) {
            auto u = get(index, source(e.second, g));
            auto v = get(index, target(e.second, g));
            if (m_min_cut.is_in_source_set(u) &&
                m_min_cut.is_in_source_set(v)) {
                expr += e.first;
            }
        }
        lp.add_row(std::move(expr) <= m_min_cut.source_set_size() - 2);
    }

  private:

    /**
     * Creates the auxiliary directed graph used for feasibility testing.
     */
    template <typename Problem, typename LP>
    void fill_auxiliary_digraph(const Problem &problem, const LP &lp) {
        auto const &g = problem.get_graph();
        auto const &index = problem.get_index();
        m_vertices_num = num_vertices(g);
        m_min_cut.init(m_vertices_num);
        m_src_to_v.resize(m_vertices_num);
        m_v_to_trg.resize(m_vertices_num);

        for (auto const &e : problem.get_edge_map().right) {
            lp::col_id col_idx = e.first;
            double col_val = lp.get_col_value(col_idx) / 2;

            if (!problem.get_compare().e(col_val, 0)) {
                auto u = get(index, source(e.second, g));
                auto v = get(index, target(e.second, g));
                m_min_cut.add_edge_to_graph(u, v, col_val, col_val);
            }
        }

        m_src = m_min_cut.add_vertex_to_graph();
        m_trg = m_min_cut.add_vertex_to_graph();

        for (auto v : boost::as_array(vertices(g))) {
            auto aux_v = get(index, v);
            m_src_to_v[aux_v] = m_min_cut
                .add_edge_to_graph(m_src, aux_v, degree_of(problem, v, lp) / 2)
                .first;
            m_v_to_trg[aux_v] =
                m_min_cut.add_edge_to_graph(aux_v, m_trg, 1).first;
        }
    }

    /**
     * Initializes the list of cut candidates.
     */
    template <typename Problem>
    void initialize_candidates(const Problem &problem) {
        auto const &g = problem.get_graph();
        auto const &index = problem.get_index();
        auto src = *(std::next(vertices(g).first, rand() % m_vertices_num));
        auto aux_src = get(index, src);
        m_candidate_list.clear();
        for (auto v : boost::as_array(vertices(g))) {
            if (v != src) {
                auto aux_v = get(index, v);
                m_candidate_list.push_back(std::make_pair(aux_src, aux_v));
                m_candidate_list.push_back(std::make_pair(aux_v, aux_src));
            }
        }
    }

    /**
     * Calculates the sum of the variables for edges incident with a given
     * vertex.
     */
    template <typename Problem, typename LP, typename Vertex>
    double degree_of(const Problem &problem, const Vertex &v, const LP &lp) {
        double res = 0;

        for (auto e : boost::as_array(out_edges(v, problem.get_graph()))) {
            auto col_id = problem.edge_to_col(e);
            if (col_id) {
                res += lp.get_col_value(*col_id);
            }
        }
        return res;
    }

    /**
     * Finds the most violated set of vertices containing \c src and not
     * containing \c trg and returns its violation value.
     * @param src vertex to be contained in the violating set
     * @param trg vertex not to be contained in the violating set
     * @return violation of the found set
     */
    double find_violation(AuxVertex src, AuxVertex trg) {
        double orig_cap = m_min_cut.get_capacity(m_src_to_v[src]);

        m_min_cut.set_capacity(m_src_to_v[src], m_vertices_num);
        // capacity of m_src_to_v[trg] does not change
        m_min_cut.set_capacity(m_v_to_trg[src], 0);
        m_min_cut.set_capacity(m_v_to_trg[trg], m_vertices_num);

        double min_cut_weight = m_min_cut.find_min_cut(m_src, m_trg);
        double violation = m_vertices_num - 1 - min_cut_weight;

        // reset the original values for the capacities
        m_min_cut.set_capacity(m_src_to_v[src], orig_cap);
        // capacity of m_src_to_v[trg] does not change
        m_min_cut.set_capacity(m_v_to_trg[src], 1);
        m_min_cut.set_capacity(m_v_to_trg[trg], 1);

        return violation;
    }

    int m_vertices_num;

    AuxVertex m_src;
    AuxVertex m_trg;

    AuxEdgeList m_src_to_v;
    AuxEdgeList m_v_to_trg;

    CandidateList m_candidate_list;

    min_cut_finder m_min_cut;
};

} //! ir
} //! paal
#endif // PAAL_BOUNDED_DEGREE_MST_ORACLE_HPP
