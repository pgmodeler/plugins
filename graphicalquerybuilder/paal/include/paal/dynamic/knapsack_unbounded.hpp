//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_unbounded.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-20
 */
#ifndef PAAL_KNAPSACK_UNBOUNDED_HPP
#define PAAL_KNAPSACK_UNBOUNDED_HPP

#include "paal/dynamic/knapsack/fill_knapsack_dynamic_table.hpp"
#include "paal/dynamic/knapsack/get_bound.hpp"
#include "paal/dynamic/knapsack/knapsack_common.hpp"
#include "paal/greedy/knapsack_unbounded_two_app.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/knapsack_utils.hpp"
#include "paal/utils/less_pointees.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/irange.hpp"

#include <boost/range/adaptor/reversed.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace paal {

namespace detail {
/**
 * @brief For knapsack dynamic algorithm for given element the table has to be
 * traversed from the lowest to highest element
 */
struct knapsack_get_position_range {
    template <typename T>
    auto operator()(T begin, T end)->decltype(irange(begin, end)) {
        return irange(begin, end);
    }
};

template <typename KnapsackData,
          typename GetBestElement, typename ValuesComparator,
          typename ReturnType = typename KnapsackData::return_type>
ReturnType knapsack_unbounded_dynamic(
    KnapsackData knap_data,
    GetBestElement getBest, ValuesComparator compareValues) {
    using Value = typename KnapsackData::value;
    using Size  = typename KnapsackData::size;
    using ObjectsIter = typename KnapsackData::object_iter;
    using ObjIterWithValueOrNull =
        boost::optional<std::pair<ObjectsIter, Value>>;
    std::vector<ObjIterWithValueOrNull> objectOnSize(knap_data.get_capacity() + 1);

    auto compare = [ = ](const ObjIterWithValueOrNull & left,
                         const ObjIterWithValueOrNull & right) {
        return compareValues(left->second, right->second);
    };

    auto objectOnSizeBegin = objectOnSize.begin();
    auto objectOnSizeEnd = objectOnSize.end();
    fill_knapsack_dynamic_table(objectOnSizeBegin, objectOnSizeEnd, knap_data.get_objects(), knap_data.get_size(),
                                [&](ObjIterWithValueOrNull val, ObjectsIter obj)
                                    ->ObjIterWithValueOrNull{
        return std::make_pair(obj, val->second + knap_data.get_value(*obj));
    },
                                compare, [](ObjIterWithValueOrNull & val) {
        val = std::make_pair(ObjectsIter{}, Value{});
    },
                                detail::knapsack_get_position_range());

    // getting position of the max value in the objectOnSize array
    auto maxPos = getBest(objectOnSizeBegin, objectOnSizeEnd, compare);

    // setting solution
    auto remainingSpaceInKnapsack = maxPos;
    while (remainingSpaceInKnapsack != objectOnSizeBegin) {
        assert(*remainingSpaceInKnapsack);
        auto && obj = *((*remainingSpaceInKnapsack)->first);
        knap_data.out(obj);
        remainingSpaceInKnapsack -= knap_data.get_size(obj);
    }

    // returning result
    if (maxPos != objectOnSizeEnd) {
        assert(*maxPos);
        return ReturnType((*maxPos)->second, maxPos - objectOnSizeBegin);
    } else {
        return ReturnType(Value{}, Size{});
    }
}

/**
 * @brief Solution to the knapsack problem
 *
 * @tparam OutputIterator
 * @param objects given objects
 * @param out the result is returned using output iterator
 * @param size functor that for given object returns its size
 * @param value functor that for given object returns its value
 */
template <typename KnapsackData,
          typename ReturnType = typename KnapsackData::return_type,
          typename Size       = typename KnapsackData::size>
ReturnType knapsack(KnapsackData knap_data,
         unbounded_tag, integral_value_tag, retrieve_solution_tag) {
    using ValueType = typename KnapsackData::value;
    using ObjectsIter = typename KnapsackData::object_iter;
    using TableElementType = boost::optional<std::pair<ObjectsIter, ValueType>>;

    auto && objects = knap_data.get_objects();

    if (boost::empty(objects)) {
        return ReturnType{};
    }
    auto maxSize = get_value_bound(knap_data, unbounded_tag{}, upper_tag{});
    auto ret = knapsack_unbounded_dynamic(
            detail::make_knapsack_data(
        knap_data.get_objects(), maxSize, knap_data.get_value(), knap_data.get_size(), knap_data.get_output_iter()),
        get_max_element_on_value_indexed_collection<TableElementType, Size>(
            TableElementType(std::make_pair(ObjectsIter{}, knap_data.get_capacity() + 1))),
        utils::greater{});
    return ReturnType(ret.second, ret.first);
}

/**
 * @brief Solution to the knapsack problem
 *
 * @tparam OutputIterator
 * @param oBegin given objects
 * @param oEnd
 * @param out the result is returned using output iterator
 * @param size functor that for given object returns its size
 * @param value functor that for given object returns its value
 */
template <typename KnapsackData>
typename KnapsackData::return_type
knapsack(KnapsackData knap_data,
         unbounded_tag, integral_size_tag, retrieve_solution_tag) {
    using Value = typename KnapsackData::value;
    return knapsack_unbounded_dynamic(std::move(knap_data),
        detail::get_max_element_on_capacity_indexed_collection<Value>(),
        utils::less{});
}

} // detail

/**
 * @brief Solution to the knapsack problem
 *
 * @tparam Objects
 * @tparam OutputIterator
 * @tparam ObjectSizeFunctor
 * @tparam ObjectValueFunctor
 * @param oBegin given objects
 * @param oEnd
 * @param out the result is returned using output iterator
 * @param size functor that for given object returns its size
 * @param value functor that for given object returns its value
 */
template <typename Objects, typename OutputIterator,
          typename ObjectSizeFunctor,
          typename ObjectValueFunctor = utils::return_one_functor>
typename detail::knapsack_base<Objects, ObjectSizeFunctor,
                               ObjectValueFunctor>::return_type
knapsack_unbounded(Objects && objects,
         detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
             capacity, // capacity is of size type
         OutputIterator out, ObjectSizeFunctor size,
         ObjectValueFunctor value = ObjectValueFunctor()) {
    return detail::knapsack_check_integrality(detail::make_knapsack_data(std::forward<Objects>(objects), capacity, size,
                                              value, out), detail::unbounded_tag{},
                                              detail::retrieve_solution_tag());
}

} // paal

#endif // PAAL_KNAPSACK_UNBOUNDED_HPP
