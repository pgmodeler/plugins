//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file paal/local_search/search_traits.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-19
 */
#ifndef PAAL_SEARCH_TRAITS_HPP
#define PAAL_SEARCH_TRAITS_HPP

#include "paal/data_structures/components/component_traits.hpp"
#include "paal/local_search/search_components.hpp"
#include "paal/utils/type_functions.hpp"

namespace paal {
namespace local_search {

/**
 * @brief Traits class for search_components
 *
 * @tparam search_components
 */
template <typename search_components> struct search_components_traits {
    typedef typename data_structures::component_traits<
        search_components>::template type<GetMoves>::type GetMovesT;
    typedef typename data_structures::component_traits<
        search_components>::template type<Gain>::type GainT;
    typedef typename data_structures::component_traits<
        search_components>::template type<Commit>::type CommitT;
};

/**
 * @brief metafunction returns move type in single_solution case
 *
 * @tparam search_components
 * @tparam Solution
 */
template <typename GetMoves, typename Solution> class move_type_from_get_moves {
    typedef typename std::remove_reference<
        typename std::result_of<GetMoves(Solution &)>::type>::type MovesRange;
    typedef typename boost::range_iterator<MovesRange>::type MoveIterator;

  public:
    typedef typename std::iterator_traits<MoveIterator>::value_type value_type;
    typedef typename std::iterator_traits<MoveIterator>::reference reference;
};

/**
 * @brief metafunction returns move type in single_solution case
 *
 * @tparam search_components
 * @tparam Solution
 */
template <typename SearchComponents, typename Solution>
struct move_type : public move_type_from_get_moves<
    typename search_components_traits<SearchComponents>::GetMovesT, Solution> {
};

/**
 * @brief metafunction returns Fitness type in single_solution case
 *
 * @tparam search_components
 * @tparam Solution
 */
template <typename Gain, typename GetMoves, typename Solution>
class fitness_from_gain_and_get_moves {
    typedef typename move_type_from_get_moves<GetMoves, Solution>::value_type
        Move;

  public:
    typedef pure_result_of_t<Gain(Solution &, Move &)> type;
};

/**
 * @brief metafunction returns Fitness type in single_solution case
 *
 * @tparam search_components
 * @tparam Solution
 */
template <typename SearchComponents, typename Solution>
using fitness_t = typename fitness_from_gain_and_get_moves<
    typename search_components_traits<SearchComponents>::GainT,
    typename search_components_traits<SearchComponents>::GetMovesT, Solution>::type;

} //! local_search
} //! paal
#endif // PAAL_SEARCH_TRAITS_HPP
