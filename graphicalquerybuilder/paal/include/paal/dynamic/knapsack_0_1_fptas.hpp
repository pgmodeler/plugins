//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_0_1_fptas.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-04
 */
#ifndef PAAL_KNAPSACK_0_1_FPTAS_HPP
#define PAAL_KNAPSACK_0_1_FPTAS_HPP

#include "paal/dynamic/knapsack_0_1.hpp"
#include "paal/dynamic/knapsack/knapsack_fptas_common.hpp"

namespace paal {

template <typename OutputIterator, typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor>
typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                               ObjectValueFunctor>::return_type
knapsack_0_1_on_value_fptas(
    double epsilon, Objects &&objects,
    detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
        capacity, // capacity is of size type
    OutputIterator out, ObjectSizeFunctor size, ObjectValueFunctor value) {
    return detail::knapsack_general_on_value_fptas_retrieve(
        epsilon, detail::make_knapsack_data(std::forward<Objects>(objects),
                                            capacity, size, value, out),
        detail::zero_one_tag{});
}

template <typename OutputIterator, typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor>
typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                               ObjectValueFunctor>::return_type
knapsack_0_1_on_size_fptas(
    double epsilon, Objects &&objects,
    detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
        capacity, // capacity is of size type
    OutputIterator out, ObjectSizeFunctor size, ObjectValueFunctor value) {
    return detail::knapsack_general_on_size_fptas_retrieve(
        epsilon, detail::make_knapsack_data(std::forward<Objects>(objects),
                                            capacity, size, value, out),
        detail::zero_one_tag{});
}

template <typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor>
typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                               ObjectValueFunctor>::return_type
knapsack_0_1_no_output_on_value_fptas(
    double epsilon, Objects &&objects,
    detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
        capacity, // capacity is of size type
    ObjectSizeFunctor size, ObjectValueFunctor value) {
    auto out = boost::make_function_output_iterator(utils::skip_functor());
    return detail::knapsack_general_on_value_fptas(
        epsilon, detail::make_knapsack_data(std::forward<Objects>(objects),
                                            capacity, size, value, out),
        detail::zero_one_tag{}, detail::no_retrieve_solution_tag{});
}

template <typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor>
typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                               ObjectValueFunctor>::return_type
knapsack_0_1_no_output_on_size_fptas(
    double epsilon, Objects &&objects,
    detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
        capacity, // capacity is of size type
    ObjectSizeFunctor size, ObjectValueFunctor value) {
    auto out = boost::make_function_output_iterator(utils::skip_functor());
    return detail::knapsack_general_on_size_fptas(
        epsilon, detail::make_knapsack_data(std::forward<Objects>(objects),
                                            capacity, size, value, out),
        detail::zero_one_tag{}, detail::no_retrieve_solution_tag{});
}

} // paal

#endif // PAAL_KNAPSACK_0_1_FPTAS_HPP
