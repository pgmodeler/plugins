//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file facility_location_add.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-08
 */
#ifndef PAAL_FACILITY_LOCATION_ADD_HPP
#define PAAL_FACILITY_LOCATION_ADD_HPP

#include "paal/utils/type_functions.hpp"
#include "paal/data_structures/facility_location/facility_location_solution_traits.hpp"
#include "paal/utils/type_functions.hpp"

#include <boost/iterator/iterator_adaptor.hpp>

#include <vector>
#include <numeric>
#include <cstdlib>
#include <cassert>

namespace paal {
namespace local_search {

/**
 * @brief commit functor for add moves in facility location problem
 */
struct facility_location_commit_add {
    /**
     * @brief operator ()
     *
     * @tparam Solution
     * @param s
     * @param e
     */
    template <typename Solution, typename UnchosenElement>
    bool operator()(Solution &s, UnchosenElement &e) // SolutionElement
        {
        s.add_facility(e);
        return true;
    }
};

/**
 * @brief gain functor for add moves in facility location problem
 */
struct facility_locationget_moves_add {

    /**
     * @brief operator()
     *
     * @tparam Solution
     *
     * @return
     */
    template <typename Solution>
    auto operator()(const Solution &sol)->decltype(sol.getUnchosenCopy()) {
        // the move of UNCHOSEN could be added to the solution
        return sol.getUnchosenCopy();
    }
};

/**
 * @brief gain functor for add moves in facility location problem
 */
struct facility_location_gain_add {
    /**
     * @brief operator()
     *
     * @tparam Solution
     * @param s
     * @param e
     *
     * @return
     */
    template <typename Solution, typename UnchosenElement>
    auto operator()(Solution &s, UnchosenElement e) const {
        auto ret = s.add_facility_tentative(e);
        auto back = s.remove_facility_tentative(e);
        assert(ret == -back);
        return -ret;
    }
};

} // local_search
} // paal

#endif // PAAL_FACILITY_LOCATION_ADD_HPP
