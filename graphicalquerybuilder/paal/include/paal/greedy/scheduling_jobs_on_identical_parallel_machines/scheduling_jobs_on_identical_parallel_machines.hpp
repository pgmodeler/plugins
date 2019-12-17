//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file scheduling_jobs_on_identical_parallel_machines.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-09-06
 */
#ifndef PAAL_SCHEDULING_JOBS_ON_IDENTICAL_PARALLEL_MACHINES_HPP
#define PAAL_SCHEDULING_JOBS_ON_IDENTICAL_PARALLEL_MACHINES_HPP

#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/irange.hpp"

#include <queue>
#include <vector>
#include <algorithm>
#include <utility>


namespace paal {
namespace greedy {
namespace detail {
class compare {
  public:
    compare(std::vector<int> &load) : m_load(load) {}
    bool operator()(int lhs, int rhs) const {
        return m_load[lhs] < m_load[rhs];
    }

  private:
    const std::vector<int> &m_load;
};
} //!detail

/**
 * @brief this is solve scheduling jobs on identical parallel machines problem
 * and return schedule
 * example:
 *  \snippet scheduling_jobs_on_identical_parallel_machines_example.cpp Scheduling Jobs On Identical Parallel Machines Example
 *
 * example file is
 * scheduling_jobs_on_identical_parallel_machines_example.cpp
 * @param n_machines
 * @param first
 * @param last
 * @param result
 * @param get_time
 */
template <class InputIterator, class OutputIterator, class GetTime>
void scheduling_jobs_on_identical_parallel_machines(int n_machines,
                                                    InputIterator first,
                                                    InputIterator last,
                                                    OutputIterator result,
                                                    GetTime get_time) {
    using JobReference =
        typename std::iterator_traits<InputIterator>::reference;
    using Time = pure_result_of_t<GetTime(JobReference)>;

    std::sort(first, last, utils::greater());
    std::vector<int> load(n_machines);

    std::priority_queue<int, std::vector<int>, detail::compare> machines(load);

    for (auto machine_id : irange(n_machines)) {
        machines.push(machine_id);
    }
    for (auto job_iter = first; job_iter < last; job_iter++) {
        int least_loaded_machine = machines.top();
        machines.pop();
        load[least_loaded_machine] -= get_time(*job_iter);
        machines.push(least_loaded_machine);
        *result = std::make_pair(least_loaded_machine, job_iter);
        ++result;
    }
}

} //!greedy
} //!paal

#endif // PAAL_SCHEDULING_JOBS_ON_IDENTICAL_PARALLEL_MACHINES_HPP
