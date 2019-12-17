//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2014 Piotr Godlewski
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file generalised_assignment_example.cpp
 * @brief
 * @author Piotr Wygocki, Piotr Godlewski
 * @version 1.0
 * @date 2013-02-04
 */

    //! [Generalised Assignment Example]
#include "paal/iterative_rounding/generalised_assignment/generalised_assignment.hpp"

#include <iostream>
#include <vector>

int main() {
    // sample problem
    std::vector<int> machines = {0,1};
    std::vector<int> jobs = {0,1};

    std::vector<std::vector<int>> cost(2, std::vector<int>(2));
    cost[0][0] = 2;
    cost[0][1] = 3;
    cost[1][0] = 1;
    cost[1][1] = 3;
    auto costf = [&](int i, int j) { return cost[i][j]; };

    std::vector<std::vector<int>> time(2, std::vector<int>(2));
    time[0][0] = 2;
    time[0][1] = 2;
    time[1][0] = 1;
    time[1][1] = 1;
    auto timef = [&](int i, int j) { return time[i][j]; };

    std::vector<int> T = { 2, 2 };
    auto Tf = [&](int i) { return T[i]; };

    std::vector<std::pair<int, int>> jobs_to_machines;

    // solve it
    auto result = paal::ir::generalised_assignment_iterative_rounding(
        machines.begin(), machines.end(), jobs.begin(), jobs.end(), costf,
        timef, Tf, std::back_inserter(jobs_to_machines));

    // print result
    if (result.first == paal::lp::OPTIMAL) {
        for (auto jm : jobs_to_machines) {
            std::cout << "Job " << jm.first << " assigned to Machine "
                << jm.second << std::endl;
        }
        std::cout << "Cost of the solution: " << *(result.second) << std::endl;
    } else {
        std::cout << "The instance is infeasible" << std::endl;
    }
    paal::lp::glp::free_env();
    return 0;
}
    //! [Generalised Assignment Example]
