//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/**
 * @file knapsack_greedy.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-07
 */

#ifndef PAAL_KNAPSACK_GREEDY_HPP
#define PAAL_KNAPSACK_GREEDY_HPP

#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/knapsack_utils.hpp"

#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/algorithm/remove_if.hpp>

namespace paal {
namespace detail {

// if the knapsack dynamic table is indexed by values,
// the procedure to find the best element is to find the biggest index i in the
// table that
// *i is smaller than given threshold(capacity)
template <typename MaxValueType, typename SizeType>
struct get_max_element_on_value_indexed_collection {
    get_max_element_on_value_indexed_collection(MaxValueType maxValue)
        : m_max_value(maxValue) {}

    template <typename Iterator, typename Comparator>
    Iterator operator()(Iterator begin, Iterator end, Comparator compare) {
        auto compareOpt = make_less_pointees_t(compare);
        // traverse in reverse order, skip the first
        for (auto iter = end - 1; iter != begin; --iter) {
            if (*iter && compareOpt(m_max_value, *iter)) {
                return iter;
            }
        }

        return end;
    }

  private:
    MaxValueType m_max_value;
};

// if the knapsack dynamic table is indexed by sizes,
// the procedure to find the best element is to find the biggest
// index i in the table that maximizes *i
template <typename ValueType>
struct get_max_element_on_capacity_indexed_collection {
    template <typename Iterator, typename Comparator>
    Iterator operator()(Iterator begin, Iterator end, Comparator compare) {
        return std::max_element(begin, end, make_less_pointees_t(compare));
    }
};

template <typename KnapsackData,
          typename Is_0_1_Tag,
          typename Value = typename KnapsackData::value,
          typename Size  = typename KnapsackData::size>
typename KnapsackData::return_type knapsack_general_two_app(
    KnapsackData knapsack_data, Is_0_1_Tag is_0_1_Tag) {

    using ObjectRef = typename KnapsackData::object_ref;

    static_assert(std::is_arithmetic<Value>::value &&
                      std::is_arithmetic<Size>::value,
                  "Size type and Value type must be arithmetic types");
    auto capacity = knapsack_data.get_capacity();

    auto bad_size = [=](ObjectRef o){return knapsack_data.get_size(o) > capacity;};

    auto objects = boost::remove_if<boost::return_begin_found>(knapsack_data.get_objects(), bad_size);

    if (boost::empty(objects)) {
        return std::pair<Value, Size>();
    }

    // finding the element with the greatest density
    auto greedyFill = get_greedy_fill(
            make_knapsack_data(
                objects, capacity,
                knapsack_data.get_size(),
                knapsack_data.get_value(),
                knapsack_data.get_output_iter()), is_0_1_Tag);

    // finding the biggest set elements with the greatest density
    // this is actually small optimization compare to original algorithm
    // note that largest is transformed iterator!
    auto largest = max_element_functor(objects, knapsack_data.get_value());

    if (*largest > std::get<0>(greedyFill)) {
        knapsack_data.out(*largest.base());
        return std::make_pair(*largest, knapsack_data.get_size(*largest.base()));
    } else {
        greedy_to_output(std::get<2>(greedyFill), knapsack_data.get_output_iter(), is_0_1_Tag);
        return std::make_pair(std::get<0>(greedyFill), std::get<1>(greedyFill));
    }
}

template <typename Range>
struct is_range_const {
    using ref = typename boost::range_reference<Range>::type;
    static const bool value = std::is_const<ref>::value ||
                     !std::is_reference<ref>::value;
};
} //! detail
} //! paal
#endif // PAAL_KNAPSACK_GREEDY_HPP
