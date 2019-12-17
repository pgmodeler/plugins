//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_0_1_two_app.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-07
 */
#ifndef PAAL_KNAPSACK_0_1_TWO_APP_HPP
#define PAAL_KNAPSACK_0_1_TWO_APP_HPP

#include "paal/utils/knapsack_utils.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/greedy/knapsack/knapsack_greedy.hpp"

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/sort.hpp>

#include <type_traits>
#include <utility>

namespace paal {

namespace detail {
template <typename KnapsackData,
          typename ObjectIter = typename KnapsackData::object_iter,
          typename ObjectRef = typename KnapsackData::object_ref,
          typename Size = typename KnapsackData::size,
          typename Value = typename KnapsackData::value>
std::tuple<Value, Size, boost::iterator_range<ObjectIter>>
get_greedy_fill(KnapsackData knap_data, zero_one_tag) {

    auto density = knap_data.get_density();
    auto compare = utils::make_functor_to_comparator(density, utils::greater{});
    // objects must be lvalue because we return a subrange of this range
    auto &objects = knap_data.get_objects();

    // finding the biggest set elements with the greatest density
    boost::sort(objects, compare);

    Value valueSum{};
    Size sizeSum{};
    auto range = boost::find_if<boost::return_begin_found>(
        objects, [ =, &sizeSum, &valueSum](ObjectRef obj) {
        auto newSize = sizeSum + knap_data.get_size(obj);
        if (newSize > knap_data.get_capacity()) {
            return true;
        }
        sizeSum = newSize;
        valueSum += knap_data.get_value(obj);
        return false;
    });
    return std::make_tuple(valueSum, sizeSum, range);
}

template <typename ObjectsRange, typename OutputIter>
void greedy_to_output(ObjectsRange range, OutputIter & out, zero_one_tag) {
    for (auto obj : range) {
        *out = obj;
        ++out;
    }
}

} //! detail

/// this version of algorithm might permute, the input range
template <typename OutputIterator, typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor,
          // this enable if assures that range can be permuted
          typename std::enable_if<
              !detail::is_range_const<Objects>::value>::type * = nullptr>
typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                               ObjectValueFunctor>::return_type
knapsack_0_1_two_app(
    Objects &&objects,
    typename detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects> capacity,
    OutputIterator out, ObjectValueFunctor value, ObjectSizeFunctor size) {
    return detail::knapsack_general_two_app(
        detail::make_knapsack_data(std::forward<Objects>(objects), capacity,
                                   size, value, out),
        detail::zero_one_tag());
}
}      //! paal
#endif // PAAL_KNAPSACK_0_1_TWO_APP_HPP
