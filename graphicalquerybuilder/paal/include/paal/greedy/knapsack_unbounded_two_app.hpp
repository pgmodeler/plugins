//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_unbounded_two_app.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-07
 */
#ifndef PAAL_KNAPSACK_UNBOUNDED_TWO_APP_HPP
#define PAAL_KNAPSACK_UNBOUNDED_TWO_APP_HPP

#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/greedy/knapsack/knapsack_greedy.hpp"

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <type_traits>
#include <utility>

namespace paal {

namespace detail {
template <typename KnapsackData,
          typename ObjectIter = typename KnapsackData::object_iter,
          typename Size = typename KnapsackData::size,
          typename Value = typename KnapsackData::value>
std::tuple<Value, Size,
           std::pair<ObjectIter, unsigned>>
get_greedy_fill(KnapsackData knap_data, unbounded_tag) {

    auto density = knap_data.get_density();
    auto most_dense_iter = max_element_functor(
            knap_data.get_objects(), density).base();

    unsigned nr = knap_data.get_capacity() / knap_data.get_size(*most_dense_iter);
    Value value_sum = Value(nr) * knap_data.get_value(*most_dense_iter);
    Size  size_sum  = Size (nr) * knap_data.get_size (*most_dense_iter);
    return std::make_tuple(value_sum, size_sum,
                           std::make_pair(most_dense_iter, nr));
}

template <typename ObjectsIterAndNr, typename OutputIter>
void greedy_to_output(ObjectsIterAndNr most_dense_iter_and_nr, OutputIter & out,
                      unbounded_tag) {
    auto nr = most_dense_iter_and_nr.second;
    auto most_dense_iter = most_dense_iter_and_nr.first;
    for (unsigned i = 0; i < nr; ++i) {
        *out = *most_dense_iter;
        ++out;
    }
}

} //! detail

///this version of algorithm might permute, the input range
template <typename OutputIterator, typename Objects,
          typename ObjectSizeFunctor, typename ObjectValueFunctor,
          //this enable if assures that range can be permuted
          typename std::enable_if<!detail::is_range_const<Objects>::value>::type * = nullptr>

typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                                  ObjectValueFunctor>::return_type
knapsack_unbounded_two_app(
    Objects && objects,
    typename detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects> capacity,
    OutputIterator out, ObjectValueFunctor value, ObjectSizeFunctor size) {
    return detail::knapsack_general_two_app(
            detail::make_knapsack_data(std::forward<Objects>(objects), capacity, size, value, out),
            detail::unbounded_tag{});
}
} //! paal
#endif // PAAL_KNAPSACK_UNBOUNDED_TWO_APP_HPP
