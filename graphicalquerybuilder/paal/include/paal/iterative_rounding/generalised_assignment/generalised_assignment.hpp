//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2014 Piotr Godlewski
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file generalised_assignment.hpp
 * @brief
 * @author Piotr Wygocki, Piotr Godlewski
 * @version 1.0
 * @date 2013-05-06
 */
#ifndef PAAL_GENERALISED_ASSIGNMENT_HPP
#define PAAL_GENERALISED_ASSIGNMENT_HPP


#include "paal/iterative_rounding/ir_components.hpp"
#include "paal/iterative_rounding/iterative_rounding.hpp"

#include <boost/range/adaptor/indexed.hpp>

namespace paal {
namespace ir {

/**
 * Relax Condition of the IR Generalised Assignment algorithm.
 */
struct ga_relax_condition {
    /**
     * Checks if a given row of the LP corresponds to a machine and can be
     * relaxed.
     */
    template <typename Problem, typename LP>
    bool operator()(Problem &problem, const LP &lp, lp::row_id row) {
        auto &&machine_rows = problem.get_machine_rows();
        if (machine_rows.find(row) == machine_rows.end()) {
            return false;
        }
        auto row_deg = lp.get_row_degree(row);
        return row_deg <= 1 || (row_deg == 2 && problem.get_compare().ge(
                                                    lp.get_row_sum(row), 1));
    }
};

/**
 * Set Solution component of the IR Generalised Assignment algorithm.
 */
struct ga_set_solution {
    /**
     * Creates the result assignment form the LP (all edges with value 1).
     */
    template <typename Problem, typename GetSolution>
    void operator()(Problem &problem, const GetSolution &solution) {
        auto jbegin = std::begin(problem.get_jobs());
        auto mbegin = std::begin(problem.get_machines());
        auto &col_idx = problem.get_col_idx();
        auto job_to_machine = problem.get_job_to_machines();

        for (auto idx : irange(col_idx.size())) {
            if (problem.get_compare().e(solution(col_idx[idx]), 1)) {
                *job_to_machine =
                    std::make_pair(*(jbegin + problem.get_j_idx(idx)),
                                   *(mbegin + problem.get_m_idx(idx)));
                ++job_to_machine;
            }
        }
    }
};

/**
 * Initialization of the IR Generalised Assignment algorithm.
 */
class ga_init {
  public:
    /**
     * Initializes the LP: variables for edges, constraints for jobs and
     * machines.
     */
    template <typename Problem, typename LP>
    void operator()(Problem &problem, LP &lp) {
        lp.set_lp_name("generalized assignment problem");
        lp.set_optimization_type(lp::MINIMIZE);

        add_variables(problem, lp);
        add_constraints_for_jobs(problem, lp);
        add_constraints_for_machines(problem, lp);
    }

  private:
    /**
     * Adds a variable to the LP for each (machine, job) edge, unless the
     * job proceeding time is greater than machine available time. Binds the
     * LP columns to the (machine, job) pairs.
     */
    template <typename Problem, typename LP>
    void add_variables(Problem &problem, LP &lp) {
        auto &col_idx = problem.get_col_idx();
        col_idx.reserve(problem.get_machines_cnt() * problem.get_jobs_cnt());
        for (auto &&j : problem.get_jobs()) {
            for (auto &&m : problem.get_machines()) {
                if (problem.get_proceeding_time()(j, m) <=
                    problem.get_machine_available_time()(m)) {
                    col_idx.push_back(lp.add_column(problem.get_cost()(j, m)));
                } else {
                    col_idx.push_back(
                        lp.add_column(problem.get_cost()(j, m), 0, 0));
                }
            }
        }
    }

    // constraints for job
    template <typename Problem, typename LP>
    void add_constraints_for_jobs(Problem &problem, LP &lp) {
        auto &col_idx = problem.get_col_idx();
        for (auto j_idx : irange(problem.get_jobs_cnt())) {
            lp::linear_expression expr;
            for (auto m_idx : irange(problem.get_machines_cnt())) {
                expr += col_idx[problem.idx(j_idx, m_idx)];
            }
            lp.add_row(std::move(expr) == 1.0);
        }
    }

    // constraints for machines
    template <typename Problem, typename LP>
    void add_constraints_for_machines(Problem &problem, LP &lp) {
        auto &col_idx = problem.get_col_idx();
        for (auto m : problem.get_machines() | boost::adaptors::indexed()) {
            auto T = problem.get_machine_available_time()(m.value());
            lp::linear_expression expr;

            for (auto j : problem.get_jobs() | boost::adaptors::indexed()) {
                auto t = problem.get_proceeding_time()(j.value(), m.value());
                auto x = col_idx[problem.idx(j.index(), m.index())];
                expr += x * t;
            }
            auto row = lp.add_row(std::move(expr) <= T);
            problem.get_machine_rows().insert(row);
        }
    }
};

template <typename Init = ga_init,
          typename RoundCondition = default_round_condition,
          typename RelaxContition = ga_relax_condition,
          typename SetSolution = ga_set_solution>
using ga_ir_components =
    IRcomponents<Init, RoundCondition, RelaxContition, SetSolution>;


/**
 * @class generalised_assignment
 * @brief The class for solving the Generalised Assignment problem using
* Iterative Rounding.
 *
 * @tparam MachineIter
 * @tparam JobIter
 * @tparam Cost
 * @tparam ProceedingTime
 * @tparam MachineAvailableTime
 * @tparam JobsToMachinesOutputIterator
 */
template <typename MachineIter, typename JobIter, typename Cost,
          typename ProceedingTime, typename MachineAvailableTime,
          typename JobsToMachinesOutputIterator>
class generalised_assignment {
  public:
    using Job = typename std::iterator_traits<JobIter>::value_type;
    using Machine = typename std::iterator_traits<MachineIter>::value_type;

    using Compare = utils::compare<double>;
    using MachineRows = std::unordered_set<lp::row_id>;
    using ColIdx = std::vector<lp::col_id>;

    using ErrorMessage = boost::optional<std::string>;

    /**
     * Constructor.
     */
    generalised_assignment(MachineIter mbegin, MachineIter mend, JobIter jbegin,
                           JobIter jend, const Cost &c, const ProceedingTime &t,
                           const MachineAvailableTime &T,
                           JobsToMachinesOutputIterator job_to_machines)
        : m_m_cnt(std::distance(mbegin, mend)),
          m_j_cnt(std::distance(jbegin, jend)), m_jbegin(jbegin), m_jend(jend),
          m_mbegin(mbegin), m_mend(mend), m_c(c), m_t(t), m_T(T),
          m_job_to_machine(job_to_machines) {}

    /**
     * Checks if input is valid.
     */
    ErrorMessage check_input_validity() {
        return ErrorMessage{};
    }

    /**
     * Returns the index of the edge between a given job and a given machine.
     */
    std::size_t idx(std::size_t j_idx, std::size_t m_idx) { return j_idx * m_m_cnt + m_idx; }

    /**
     * Returns the index of a job given the index of the edge between the job
     * and a machine.
     */
    std::size_t get_j_idx(std::size_t idx) { return idx / m_m_cnt; }

    /**
     * Returns the index of a machine given the index of the edge between a job
     * and the machine.
     */
    std::size_t get_m_idx(std::size_t idx) { return idx % m_m_cnt; }

    /**
     * Returns the LP rows corresponding to the machines.
     */
    MachineRows &get_machine_rows() { return m_machine_rows; }

    /**
     * Returns the double comparison object.
     */
    Compare get_compare() { return m_compare; }

    /**
     * Returns the number of machines in the problem.
     */
    std::size_t get_machines_cnt() const { return m_m_cnt; }

    /**
     * Returns the number of jobs in the problem.
     */
    std::size_t get_jobs_cnt() const { return m_j_cnt; }

    /**
     * Returns the machines iterator range.
     */
    boost::iterator_range<MachineIter> get_machines() {
        return boost::make_iterator_range(m_mbegin, m_mend);
    }

    /**
     * Returns the jobs iterator range.
     */
    boost::iterator_range<JobIter> get_jobs() {
        return boost::make_iterator_range(m_jbegin, m_jend);
    }

    /**
     * Returns the vector of LP column IDs.
     */
    ColIdx &get_col_idx() { return m_col_idx; }

    /**
     * Returns the result output iterator.
     */
    JobsToMachinesOutputIterator get_job_to_machines() {
        return m_job_to_machine;
    }

    /**
     * Returns the proceeding time function (function from (job, machine)
     * pairs into the proceeding time of the job on the machine).
     */
    const ProceedingTime &get_proceeding_time() { return m_t; }

    /**
     * Returns the machine available time function (function returning
     * the time available on a given machine).
     */
    const MachineAvailableTime &get_machine_available_time() { return m_T; }

    /**
     * Returns the cost function (function from (job, machine)
     * pairs into the cost of executing the job on the machine).
     */
    const Cost &get_cost() const { return m_c; }

  private:

    const std::size_t m_m_cnt;
    const std::size_t m_j_cnt;
    JobIter m_jbegin;
    JobIter m_jend;
    MachineIter m_mbegin;
    MachineIter m_mend;
    const Cost &m_c;
    const ProceedingTime &m_t;
    const MachineAvailableTime &m_T;
    JobsToMachinesOutputIterator m_job_to_machine;
    const Compare m_compare;
    ColIdx m_col_idx;
    MachineRows m_machine_rows;
};

/**
 * @brief Creates a generalised_assignment object.
 *
 * @tparam MachineIter
 * @tparam JobIter
 * @tparam Cost
 * @tparam ProceedingTime
 * @tparam MachineAvailableTime
 * @tparam JobsToMachinesOutputIterator
 * @param mbegin begin machines iterator
 * @param mend end machines iterator
 * @param jbegin begin jobs iterator
 * @param jend end jobs iterator
 * @param c costs of assignments
 * @param t jobs proceeding times
 * @param T times available for the machines
 * @param jobs_to_machines found assignment
 *
 * @return generalised_assignment object
 */
template <typename MachineIter, typename JobIter, typename Cost,
          typename ProceedingTime, typename MachineAvailableTime,
          typename JobsToMachinesOutputIterator>
generalised_assignment<MachineIter, JobIter, Cost, ProceedingTime,
                       MachineAvailableTime, JobsToMachinesOutputIterator>
make_generalised_assignment(MachineIter mbegin, MachineIter mend,
                            JobIter jbegin, JobIter jend, const Cost &c,
                            const ProceedingTime &t,
                            const MachineAvailableTime &T,
                            JobsToMachinesOutputIterator jobs_to_machines) {
    return generalised_assignment<
        MachineIter, JobIter, Cost, ProceedingTime, MachineAvailableTime,
        JobsToMachinesOutputIterator>(mbegin, mend, jbegin, jend, c, t, T,
                                      jobs_to_machines);
}

/**
 * @brief Solves the Generalised Assignment problem using Iterative Rounding.
 *
 * @tparam MachineIter
 * @tparam JobIter
 * @tparam Cost
 * @tparam ProceedingTime
 * @tparam MachineAvailableTime
 * @tparam JobsToMachinesOutputIterator
 * @tparam Components
 * @tparam Visitor
 * @param mbegin begin machines iterator
 * @param mend end machines iterator
 * @param jbegin begin jobs iterator
 * @param jend end jobs iterator
 * @param c costs of assignments
 * @param t jobs proceeding times
 * @param T times available for the machines
 * @param jobs_to_machines found assignment
 * @param components IR components
 * @param visitor
 *
 * @return solution status
 */
template <typename MachineIter, typename JobIter, typename Cost,
          typename ProceedingTime, typename MachineAvailableTime,
          typename JobsToMachinesOutputIterator,
          typename Components = ga_ir_components<>,
          typename Visitor = trivial_visitor>
IRResult generalised_assignment_iterative_rounding(
    MachineIter mbegin, MachineIter mend, JobIter jbegin, JobIter jend,
    const Cost &c, const ProceedingTime &t, const MachineAvailableTime &T,
    JobsToMachinesOutputIterator jobs_to_machines,
    Components components = Components(), Visitor visitor = Visitor()) {
    auto ga_solution = make_generalised_assignment(mbegin, mend, jbegin, jend,
                                                   c, t, T, jobs_to_machines);
    return solve_iterative_rounding(ga_solution, std::move(components),
                                    std::move(visitor));
}


} // ir
} // paal
#endif // PAAL_GENERALISED_ASSIGNMENT_HPP
