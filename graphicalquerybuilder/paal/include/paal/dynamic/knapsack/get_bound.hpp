//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file get_bound.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-04
 */
#ifndef PAAL_GET_BOUND_HPP
#define PAAL_GET_BOUND_HPP

#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/knapsack_utils.hpp"
#include "paal/greedy/knapsack/knapsack_greedy.hpp"

#include <boost/function_output_iterator.hpp>

namespace paal {
namespace detail {

template <typename SizeType, typename ValueType>
using GetIntegralTag = typename std::conditional<
    std::is_integral<SizeType>::value && std::is_integral<ValueType>::value,
    integral_value_and_size_tag,
    typename std::conditional<
        std::is_integral<SizeType>::value, integral_size_tag,
        typename std::conditional<
            std::is_integral<ValueType>::value, integral_value_tag,
            non_integral_value_and_size_tag>::type>::type>::type;

template <typename SizeType>
using Getarithmetic_size_tag = typename std::conditional<
    std::is_arithmetic<SizeType>::value, arithmetic_size_tag,
    Nonarithmetic_size_tag>::type;

// this overloads checks if SizeType and ValueType are integral

struct upper_tag{};
struct lower_tag{};

/**
 * @brief upper bound is computed as biggest density times capacity +
 *        values for all elements with size 0. It is correct upper bound for 0/1.
 *        For unbounded case there will be no elements with size 0.
 */
template <typename KnapsackData >
typename KnapsackData::value get_density_based_value_upper_bound(KnapsackData knap_data) {
    using Size      = typename KnapsackData::size;
    using ObjectRef = typename KnapsackData::object_ref;
    auto density = knap_data.get_density();

    // this filters are really needed only in 0/1 case
    // in unbounded case, there is a guarantee that sizes are not 0
    auto not_zero_sizel = [=](ObjectRef obj) {return knap_data.get_size(obj) > Size{};};
    auto not_zero_size = utils::make_assignable_functor(not_zero_sizel);
    auto zeroSize = utils::make_not_functor(not_zero_size);

    auto not_zeros = knap_data.get_objects() | boost::adaptors::filtered(not_zero_size);
    auto zeros     = knap_data.get_objects() | boost::adaptors::filtered(zeroSize     );

    auto maxElement = *max_element_functor(not_zeros, density);
    return knap_data.get_capacity() * maxElement + sum_functor(zeros, knap_data.get_value());
}

//non-arithmetic size, upper bound
template <typename KnapsackData,
          typename Is_0_1_Tag>
typename KnapsackData::value get_value_bound(
                      KnapsackData knap_data,
                      Nonarithmetic_size_tag, Is_0_1_Tag, upper_tag) {
    return get_density_based_value_upper_bound(std::move(knap_data));
}

//arithmetic size, upper bound
template <typename KnapsackData, typename Is_0_1_Tag>
typename KnapsackData::value get_value_bound(
                      KnapsackData knap_data,
                      arithmetic_size_tag, Is_0_1_Tag is_0_1_Tag, upper_tag) {
    return std::min(2 * get_value_bound(knap_data, is_0_1_Tag, lower_tag{}),
                    get_density_based_value_upper_bound(knap_data));
}

//non-arithmetic size, lower bound
template <typename KnapsackData, typename Is_0_1_Tag>
typename KnapsackData::value get_value_bound(KnapsackData knap_data,
                      Nonarithmetic_size_tag, Is_0_1_Tag, lower_tag) {
    //computes lower bound as value of the most valuable element
    return *max_element_functor(knap_data.get_objects(), knap_data.get_value()).base();
}

//arithmetic size, lower bound
template <typename KnapsackData, typename Is_0_1_Tag>
typename KnapsackData::value get_value_bound(KnapsackData knap_data,
                      arithmetic_size_tag, Is_0_1_Tag is_0_1_Tag, lower_tag) {
    auto out = boost::make_function_output_iterator(utils::skip_functor{});
    return knapsack_general_two_app(detail::make_knapsack_data(knap_data.get_objects(),
            knap_data.get_capacity(), knap_data.get_size(), knap_data.get_value(), out), is_0_1_Tag).first;
}

//decide whether size is arithmetic or not
template <typename KnapsackData, typename Is_0_1_Tag, typename BoundType>
typename KnapsackData::value get_value_bound(KnapsackData knap_data,
                      Is_0_1_Tag is_0_1_tag, BoundType bound_type_tag) {
    return get_value_bound(std::move(knap_data),
                                 Getarithmetic_size_tag<typename KnapsackData::size>{},
                                 is_0_1_tag, bound_type_tag);
}

} //! detail
} //! paal
#endif // PAAL_GET_BOUND_HPP
