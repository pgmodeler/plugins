//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file scheduling_jobs_with_deadlines_on_a_single_machine.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-09-09
 */
#ifndef PAAL_SCHEDULING_JOBS_WITH_DEADLINES_ON_A_SINGLE_MACHINE_HPP
#define PAAL_SCHEDULING_JOBS_WITH_DEADLINES_ON_A_SINGLE_MACHINE_HPP

#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/assign_updates.hpp"

#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/algorithm/sort.hpp>

#include <queue>
#include <vector>
#include <algorithm>
#include <utility>

namespace paal {
namespace greedy {

/**
 * @brief solve scheduling jobs on identical parallel machines problem
 * and fill start time of all jobs
 * example:
 * \snippet scheduling_jobs_with_deadlines_on_a_single_machine_example.cpp Scheduling Jobs On Single Machine Example
 * example file is
 * scheduling_jobs_with_deadlines_on_a_single_machine_example.cpp
 * @param first - jobs begin
 * @param last - jobs end
 * @param get_time
 * @param get_release_date
 * @param get_due_date
 * @param result
 * @tparam Time
 * @tparam InputIterator
 * @tparam OutputIterator
 * @tparam GetTime
 * @tparam GetDueDate
 * @tparam GetReleaseDate
 */
template <class InputIterator, class OutputIterator, class GetTime,
          class GetDueDate, class GetReleaseDate>
auto scheduling_jobs_with_deadlines_on_a_single_machine(
    const InputIterator first, const InputIterator last, GetTime get_time,
    GetReleaseDate get_release_date, GetDueDate get_due_date,
    OutputIterator result) {
    using Time = puretype(get_time(*first));
    std::vector<InputIterator> jobs;
    std::copy(boost::make_counting_iterator(first),
              boost::make_counting_iterator(last), std::back_inserter(jobs));

    auto get_due_date_from_iterator =
        utils::make_lift_iterator_functor(get_due_date);
    auto due_date_compatator = utils::make_functor_to_comparator(
        get_due_date_from_iterator, utils::greater{});
    using QueueType = std::priority_queue<
        InputIterator, std::vector<InputIterator>, decltype(due_date_compatator)>;
    QueueType active_jobs_iters(due_date_compatator);

    auto get_release_date_from_iterator =
        utils::make_lift_iterator_functor(get_release_date);
    boost::sort(jobs,
              utils::make_functor_to_comparator(get_release_date_from_iterator));
    Time start_idle = Time();
    Time longest_delay = Time();
    auto do_job = [&]() {
        auto job_iter = active_jobs_iters.top();
        active_jobs_iters.pop();
        Time start_time = std::max(start_idle, get_release_date(*job_iter));
        start_idle = start_time + get_time(*job_iter);
        assign_max(longest_delay, start_idle - get_due_date(*job_iter));
        *result = std::make_pair(job_iter, start_time);
        ++result;
    };
    for (auto job_iter : jobs) {
        while (!active_jobs_iters.empty() &&
               get_release_date(*job_iter) > start_idle)
            do_job();
        active_jobs_iters.push(job_iter);
    }
    while (!active_jobs_iters.empty()) {
        do_job();
    }

    return longest_delay;
}

} //!greedy
} //!paal

#endif // PAAL_SCHEDULING_JOBS_WITH_DEADLINES_ON_A_SINGLE_MACHINE_HPP
