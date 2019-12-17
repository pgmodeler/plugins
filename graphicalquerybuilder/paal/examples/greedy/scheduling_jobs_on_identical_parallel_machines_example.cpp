//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file scheduling_jobs_on_identical_parallel_machines_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-09-06
 */

//! [Scheduling Jobs On Identical Parallel Machines Example]

#include "paal/greedy/scheduling_jobs_on_identical_parallel_machines/scheduling_jobs_on_identical_parallel_machines.hpp"

#include <iostream>
#include <utility>

/**
 * \brief scheduling jobs on identical parallel machines example
 */
int main() {
    typedef double Time;
    typedef std::pair<Time, char> Job;

    auto returnJobTimeFunctor = [](Job job) { return job.first; };

    // sample data
    int numberOfMachines = 3;
    std::vector<Job> jobs = { { 2.1, 'a' },
                              { 3.1, 'b' },
                              { 4.1, 'c' },
                              { 5.1, 'd' },
                              { 6.1, 'e' },
                              { 7.1, 'f' },
                              { 8.1, 'g' } };
    std::vector<std::pair<int, decltype(jobs)::iterator>> result;

    paal::greedy::scheduling_jobs_on_identical_parallel_machines(
        numberOfMachines, jobs.begin(), jobs.end(), back_inserter(result),
        returnJobTimeFunctor);

    std::vector<Time> sumOfMachine;
    sumOfMachine.resize(numberOfMachines);
    for (auto machineJobPair : result) {
        auto machine = machineJobPair.first;
        auto job = machineJobPair.second;
        sumOfMachine[machine] += job->first;
        std::cout << "On machine: " << machine << " do job: " << job->second
                  << std::endl;
    }
    Time maximumLoad =
        *std::max_element(sumOfMachine.begin(), sumOfMachine.end());

    // print result
    std::cout << "Solution:" << maximumLoad << std::endl;
    return 0;
}

//! [Scheduling Jobs On Identical Parallel Machines Example]
