//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file scheduling_jobs_with_deadlines_on_a_single_machine_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-09-17
 */

//! [Scheduling Jobs On Single Machine Example]

#include "paal/greedy/scheduling_jobs_with_deadlines_on_a_single_machine/scheduling_jobs_with_deadlines_on_a_single_machine.hpp"
#include "paal/utils/irange.hpp"

#include <iostream>
#include <utility>

int main() {
    // sample data
    typedef double Time;
    std::vector<Time> time = { 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1 };
    std::vector<Time> release = { 1, 2, 3, 4, 5, 6, 7 };
    std::vector<Time> due_date = { -1, 0, -2, -3, -4, -5, -6 };

    auto jobs = paal::irange(time.size());

    std::vector<std::pair<decltype(jobs)::iterator, Time>> jobs_to_start_dates;

    Time delay =
        paal::greedy::scheduling_jobs_with_deadlines_on_a_single_machine(
            jobs.begin(), jobs.end(), [&](int i) { return time[i]; },
            [&](int i) { return release[i]; },
            [&](int i) { return due_date[i]; },
            back_inserter(jobs_to_start_dates));
    for (auto job_start_time : jobs_to_start_dates) {
        std::cout << "Job " << (*job_start_time.first)
                  << " Start time: " << job_start_time.second << std::endl;
    }
    // print result
    std::cout << "Solution: " << delay << std::endl;
    return 0;
}
//! [Scheduling Jobs On Single Machine Example]
