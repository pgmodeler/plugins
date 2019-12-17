//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file scheduling_jobs_example.cpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2013-11-19
 */

//! [Scheduling Jobs Example]
#include "paal/greedy/scheduling_jobs/scheduling_jobs.hpp"
#include "paal/utils/functors.hpp"

#include <iostream>
#include <unordered_map>
#include <utility>

typedef double Time;
typedef std::pair<Time, char> Job;
typedef int Machine;

template <class Result> void print_result(const Result &result) {
    std::unordered_map<Machine, Time> machineTime;
    for (auto const &machineJobpair : result) {
        Machine machine = *machineJobpair.first;
        Job job = *machineJobpair.second;
        machineTime[machine] += job.first / machine;
        std::cout << "On machine: " << machine << " do job: " << job.second
                  << std::endl;
    }
    Time max_time = 0;
    for (auto const &it : machineTime) {
        max_time = std::max(max_time, it.second);
    }
    std::cout << "Solution: " << max_time << std::endl;
}

int main() {
    auto returnJobLoadFunctor = [](Job job) { return job.first; };

    std::vector<Machine> machines = { 1, 2, 3 };
    std::vector<Job> jobs = { { 2.1, 'a' }, { 3.1, 'b' }, { 4.1, 'c' },
                              { 5.1, 'd' }, { 6.1, 'e' }, { 7.1, 'f' },
                              { 8.1, 'g' } };

    std::vector<std::pair<decltype(machines) ::iterator,
                          decltype(jobs) ::iterator>> deterministicResult,
        randomizedResult;

    std::cout << "Deterministic schedule:" << std::endl;
    paal::greedy::schedule_deterministic(
        machines.begin(), machines.end(), jobs.begin(), jobs.end(),
        back_inserter(deterministicResult), paal::utils::identity_functor(),
        returnJobLoadFunctor);
    print_result(deterministicResult);

    std::cout << "Randomized schedule:" << std::endl;
    paal::greedy::schedule_randomized(
        machines.begin(), machines.end(), jobs.begin(), jobs.end(),
        back_inserter(randomizedResult), paal::utils::identity_functor(),
        returnJobLoadFunctor);
    print_result(randomizedResult);

    return 0;
}
//! [Scheduling Jobs Example]
