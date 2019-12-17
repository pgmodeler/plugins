//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file 2_local_search.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#ifndef PAAL_2_LOCAL_SEARCH_HPP
#define PAAL_2_LOCAL_SEARCH_HPP

#include "paal/local_search/search_components.hpp"
#include "paal/local_search/local_search.hpp"
#include "paal/local_search/2_local_search/2_local_search_components.hpp"
#include "paal/local_search/2_local_search/2_local_search_solution_adapter.hpp"
#include "paal/data_structures/cycle/cycle_start_from_last_change.hpp"
#include "paal/data_structures/cycle/cycle_concept.hpp"

namespace paal {
namespace local_search {

/**
 @brief represents step of 2 local search in multi solution where Solution is
Cycle, SolutionElement is pair of vertices and Move type is pair of vertices.
 See \ref local_search_page. There are three ways to provide search components
 <ul>
       <li> use (TODO link does not generate)
get_default_two_local_components(const Metric &) - this is the easiest way.
       <li> use TwoLocalcomponents to provide your own search components
       <li> write your own implementation of Multisearch_components
 </ul>
 Basic usage of this algorithm is extremely simple and elegant. <br> We are
using some helper functions from the library.

 \snippet 2_local_search_example.cpp Two Local Search Example

 Although the basic usage is very simple, the sophisticated user can still
easily change default parameters and exchange them with his ones. <br>
*
* @tparam Cycle input cycle, hast to be model of the  \ref cycle concept
* @tparam search_components this is model Multisearch_components
*/

template <typename... Args>
using TwoLocalcomponents = data_structures::components<
    Gain, data_structures::NameWithDefault<GetMoves, two_local_searchget_moves>,
    data_structures::NameWithDefault<Commit, two_local_search_commit>>::type<
    Args...>;

/**
 * @brief make template function for TwoLocalcomponents, just to avoid providing
* type names in template.
 *
 * @tparam Gain
 * @tparam GetMoves
 * @param ch
 * @param gm
 *
 * @return
 */
template <typename Gain, typename GetMoves = two_local_searchget_moves>
auto make_two_local_search_components(Gain ch, GetMoves gm = GetMoves{}) {
    return TwoLocalcomponents<Gain, GetMoves>(std::move(ch), std::move(gm));
}

/**
 * @brief get default two local search components
 *
 * @tparam Metric is model of \ref metric concept
 * @param m metric
 */
template <typename Metric>
auto get_default_two_local_components(const Metric &m) {
    return make_two_local_search_components(gain_two_opt<Metric>(m));
}

/**
 * @brief local search for two - opt in tsp adapts tsp to
* local_search_multi_solution
 *
 * @tparam SearchStrategy
 * @tparam ContinueOnSuccess
 * @tparam ContinueOnFail
 * @tparam Cycle
 * @tparam components
 * @param cycle
 * @param searchStrategy
 * @param on_success
 * @param on_fail
 * @param comps
 *
 * @return
 */
template <typename SearchStrategy, typename ContinueOnSuccess,
          typename ContinueOnFail, typename Cycle, typename... components>
bool two_local_search(Cycle &cycle, SearchStrategy searchStrategy,
                      ContinueOnSuccess on_success, ContinueOnFail on_fail,
                      components... comps) {
    typedef data_structures::cycle_start_from_last_change<Cycle> CSFLCh;
    CSFLCh cycleSFLCh(cycle);
    local_search::two_local_search_adapter<CSFLCh>
        cycleAdapted(cycleSFLCh);
    return local_search(cycleAdapted, std::move(searchStrategy),
                        std::move(on_success), std::move(on_fail),
                        std::move(comps)...);
}

/**
 * @brief simple version of two_local_search
 *
 * @tparam Cycle
 * @tparam components
 * @param cycle
 * @param comps
 *
 * @return
 */
template <typename Cycle, typename... components>
bool tsp_first_improving(Cycle &cycle, components... comps) {
    return two_local_search(cycle, first_improving_strategy{},
                            utils::always_true{}, utils::always_false{},
                            std::move(comps)...);
}

} // local_search
} // paal

#endif // PAAL_2_LOCAL_SEARCH_HPP
