//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file trivial_solution_commit.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-11
 */
#ifndef PAAL_TRIVIAL_SOLUTION_COMMIT_HPP
#define PAAL_TRIVIAL_SOLUTION_COMMIT_HPP

/**
 * @brief Used in case when update is actually the new solution
 */
struct trivial_commit {
    /**
     * @brief We assume that operator() receives a new solution
     *
     * @tparam Solution
     * @param s
     * @param u
     */
    template <typename Solution>
    bool operator()(Solution &s, const Solution &u) const {
        s = u;
        return true;
    }
};
#endif // PAAL_TRIVIAL_SOLUTION_COMMIT_HPP
