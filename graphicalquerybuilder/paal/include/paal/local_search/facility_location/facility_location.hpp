//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file facility_location.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#ifndef PAAL_FACILITY_LOCATION_HPP
#define PAAL_FACILITY_LOCATION_HPP

#include "facility_location_solution_adapter.hpp"
#include "facility_location_add.hpp"
#include "facility_location_remove.hpp"
#include "facility_location_swap.hpp"

#include "paal/data_structures/facility_location/facility_location_solution.hpp"
#include "paal/local_search/local_search.hpp"

namespace paal {
namespace local_search {

/**
 * @class default_remove_fl_components
 * @brief Model of Multisearch_components with default multi search components
 * for facility location.
 */
using default_remove_fl_components =
    Multisearch_components<facility_locationget_moves_remove,
                           facility_location_gain_remove,
                           facility_location_commit_remove>;

/**
 * @brief add components for facility location
 */
using default_add_fl_components =
    Multisearch_components<facility_locationget_moves_add,
                           facility_location_gain_add,
                           facility_location_commit_add>;

/**
 * @brief Swap components for facility location
 *
 */
using default_swap_fl_components =
    Multisearch_components<facility_locationget_moves_swap,
                           facility_location_gain_swap,
                           facility_location_commit_swap>;

/**
 * facility_location_local_search
 * @brief this is model of LocalSearchStepMultiSolution concept. See \ref
 local_search_page.<br>
 * The Move is facility_location::Move. <br>
 * The Solution is adapted data_structures::facility_location_solution. <br>
 * The SolutionElement is facility_location::Facility  <br>
 * Use DefaultFLcomponents for default search components.
 *
 * The FacilityLocationLocalSearchStep takes as constructor parameter
 data_structures::facility_location_solution.
 * <b> WARNING </b>
 * get_solution of the FacilityLocationLocalSearchStep returns type
 object_with_copy<facility_location_solution>.
 * If you want to perform search, then change the solution object and continue
 local search you should perform all the operations on object_with_copy. <br>
 * example:
    \snippet facility_location_example.cpp FL Search Example
 *
 * example file is facility_location_example.cpp
 *
 * @tparam voronoi
 * @tparam FacilityCost
 * @tparam Multisearch_components
 */
template <typename SearchStrategy, typename ContinueOnSuccess,
          typename ContinueOnFail, typename facility_location_solution,
          typename... components>
bool facility_location_local_search(facility_location_solution &fls,
                                    SearchStrategy searchStrategy,
                                    ContinueOnSuccess on_success,
                                    ContinueOnFail on_fail,
                                    components... comps) {
    typedef facility_location_solution_adapter<facility_location_solution> FLSA;
    FLSA flsa(fls);
    return local_search(flsa, std::move(searchStrategy), std::move(on_success),
                        std::move(on_fail), std::move(comps)...);
}

/**
 * @brief simple version of local search for facility location
 *
 * @tparam facility_location_solution
 * @tparam components
 * @param fls
 * @param comps
 *
 * @return
 */
template <typename facility_location_solution, typename... components>
bool facility_location_first_improving(facility_location_solution &fls,
                                       components... comps) {
    return facility_location_local_search(
        fls, first_improving_strategy{}, utils::always_true{},
        utils::always_false{}, std::move(comps)...);
}
} //! local_search
} //! paal

#endif // PAAL_FACILITY_LOCATION_HPP
