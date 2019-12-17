//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_common.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-30
 */
#ifndef PAAL_KNAPSACK_COMMON_HPP
#define PAAL_KNAPSACK_COMMON_HPP

#include "paal/dynamic/knapsack/get_bound.hpp"

namespace paal {
namespace detail {

template <typename KnapsackData, typename Is_0_1_Tag,
          typename RetrieveSolution = retrieve_solution_tag,
          typename Value = typename KnapsackData::value,
          typename Size = typename KnapsackData::size>
typename KnapsackData::return_type knapsack_check_integrality(KnapsackData knap_data, Is_0_1_Tag is_0_1_Tag,
                           RetrieveSolution retrieve_solutionTag =
                               RetrieveSolution{}) {

    return knapsack(std::move(knap_data), is_0_1_Tag,
                detail::GetIntegralTag<Size, Value>{},
                retrieve_solutionTag);
}

// this overloads is for nonintegral SizeType and ValueType
// this case is invalid and allwas asserts!
template <typename KnapsackData,
          typename IntegralTag, // always equals non_integral_value_and_size_tag
          typename RetrieveSolution, typename Is_0_1_Tag,
          typename = typename std::enable_if<std::is_same<
              non_integral_value_and_size_tag, IntegralTag>::value>::type>

typename KnapsackData::return_type knapsack(KnapsackData, Is_0_1_Tag is_0_1_Tag, IntegralTag,
         RetrieveSolution retrieve_solution) {
    // trick to avoid checking assert on template definition parse
    static_assert(
        std::is_same<IntegralTag, non_integral_value_and_size_tag>::value,
        "At least one of the value or size must return integral value");
}

/**
 * @brief Solution to Knapsack  problem
 *  overload for integral Size and Value case
 */
template <typename KnapsackData, typename Is_0_1_Tag, typename RetrieveSolution>
typename KnapsackData::return_type knapsack(KnapsackData knap_data, Is_0_1_Tag is_0_1_Tag,
         integral_value_and_size_tag, RetrieveSolution retrieve_solutionTag) {
    if (get_value_bound(knap_data, is_0_1_Tag, upper_tag{}) >
        knap_data.get_capacity()) {
        return knapsack(std::move(knap_data), is_0_1_Tag, integral_size_tag{},
                        retrieve_solutionTag);
    } else {
        return knapsack(std::move(knap_data), is_0_1_Tag, integral_value_tag{},
                        retrieve_solutionTag);
    }
}

}      //! detail
}      //! paal
#endif // PAAL_KNAPSACK_COMMON_HPP
