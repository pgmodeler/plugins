//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_unbounded_fptas.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-01
 */
#ifndef PAAL_KNAPSACK_UNBOUNDED_FPTAS_HPP
#define PAAL_KNAPSACK_UNBOUNDED_FPTAS_HPP

#include "paal/dynamic/knapsack_unbounded.hpp"
#include "paal/dynamic/knapsack/get_bound.hpp"
#include "paal/dynamic/knapsack/knapsack_fptas_common.hpp"

#include <boost/function_output_iterator.hpp>

namespace paal {

template <typename OutputIterator, typename Objects,
          typename ObjectSizeFunctor, typename ObjectValueFunctor>
typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                               ObjectValueFunctor>::return_type
knapsack_unbounded_on_value_fptas(
    double epsilon, Objects && objects,
    detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
        capacity, // capacity is of size type
    OutputIterator out, ObjectSizeFunctor size, ObjectValueFunctor value) {
    return detail::knapsack_general_on_value_fptas_retrieve(
        epsilon, detail::make_knapsack_data(std::forward<Objects>(objects), capacity, size, value, out),
        detail::unbounded_tag{});
}

template <typename OutputIterator, typename Objects,
          typename ObjectSizeFunctor, typename ObjectValueFunctor>
typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                               ObjectValueFunctor>::return_type
knapsack_unbounded_on_size_fptas(
    double epsilon, Objects && objects,
    detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
        capacity, // capacity is of size type
    OutputIterator out, ObjectSizeFunctor size, ObjectValueFunctor value) {
    return detail::knapsack_general_on_size_fptas_retrieve(
        epsilon, detail::make_knapsack_data(std::forward<Objects>(objects), capacity, size, value, out),
        detail::unbounded_tag{});
}

} //! paal

#endif // PAAL_KNAPSACK_UNBOUNDED_FPTAS_HPP
