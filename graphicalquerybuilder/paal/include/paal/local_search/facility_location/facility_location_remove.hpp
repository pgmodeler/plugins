//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file facility_location_remove.hpp
* @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-08
 */
#ifndef PAAL_FACILITY_LOCATION_REMOVE_HPP
#define PAAL_FACILITY_LOCATION_REMOVE_HPP

#include "paal/utils/type_functions.hpp"
#include "paal/data_structures/facility_location/facility_location_solution_traits.hpp"

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <cassert>
#include <vector>
#include <numeric>
#include <cstdlib>

namespace paal {
namespace local_search {

/**
 * @brief gain functor for facility location
 *
 */
struct facility_location_gain_remove {
    /**
     * @brief operator()
     *
     * @tparam Solution
     * @param s
     * @param e
     *
     * @return
     */
    template <typename Solution, typename ChosenElement>
    auto operator()(Solution &s, ChosenElement e) {
        auto ret = s.remove_facility_tentative(e);
        // TODO for capacitated version we should just restart copy
        auto back = s.add_facility_tentative(e);
        assert(ret == -back);
        return -ret;
    }
};

/**
 * @brief commit functor for facility location
 *
 */
struct facility_location_commit_remove {
    /**
     * @brief operator()
     *
     * @tparam Solution
     * @param s
     * @param e
     */
    template <typename Solution, typename ChosenElement>
    bool operator()(Solution &s, ChosenElement e) {
        s.remove_facility(e);
        return true;
    }
};

/**
 * @brief get moves functor for facility location remove
 *
 */
struct facility_locationget_moves_remove {
    /**
     * @brief operator()
     *
     * @tparam Solution
     *
     * @return
     */
    template <typename Solution>
    auto operator()(const Solution &sol) {
        return sol.getChosenCopy();
    }
};

} // local_search
} // paal

#endif // PAAL_FACILITY_LOCATION_REMOVE_HPP
