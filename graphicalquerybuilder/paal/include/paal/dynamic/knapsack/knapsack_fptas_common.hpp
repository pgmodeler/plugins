//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_fptas_common.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-04
 */
#ifndef PAAL_KNAPSACK_FPTAS_COMMON_HPP
#define PAAL_KNAPSACK_FPTAS_COMMON_HPP

#include "paal/utils/accumulate_functors.hpp"
#include "paal/dynamic/knapsack/get_bound.hpp"

namespace paal {
namespace detail {

/**
 * @brief computes multiplier for FPTAS, version for 0/1
 */
template <typename Objects, typename Functor>
boost::optional<double> get_multiplier(Objects &&objects, double epsilon,
                                       double lowerBound, Functor,
                                       detail::zero_one_tag) {
    double n = boost::distance(objects);
    auto ret = n / (epsilon * lowerBound);
    static const double SMALLEST_MULTIPLIER = 1.;
    if (ret > SMALLEST_MULTIPLIER) return boost::none;
    return ret;
}

// TODO this multiplier does not guarantee fptas
/**
 * @brief computes multiplier for FPTAS, unbounded version
 *
 */
template <typename Objects, typename Functor>
boost::optional<double> get_multiplier(Objects &&objects, double epsilon,
                                       double lowerBound, Functor f,
                                       detail::unbounded_tag) {
    double minF = *min_element_functor(objects, f);
    double n = int(double(lowerBound) * (1. + epsilon) / minF +
                   1.); // maximal number of elements in the found solution
    auto ret = n / (epsilon * lowerBound);
    static const double SMALLEST_MULTIPLIER = 1.;
    if (ret > SMALLEST_MULTIPLIER) return boost::none;
    return ret;
}

template <typename KnapsackData, typename IsZeroOne, typename RetrieveSolution,
          typename ReturnType = typename KnapsackData::return_type>
ReturnType knapsack_general_on_value_fptas(double epsilon,
                                           KnapsackData knap_data,
                                           IsZeroOne is_0_1_Tag,
                                           RetrieveSolution retrieve_solution) {
    using ObjectRef = typename KnapsackData::object_ref;
    using Value = typename KnapsackData::value;
    using Size = typename KnapsackData::size;

    auto &&objects = knap_data.get_objects();

    if (boost::empty(objects)) {
        return ReturnType{};
    }

    double maxValue =
        detail::get_value_bound(knap_data, is_0_1_Tag, lower_tag{});
    auto multiplier = get_multiplier(objects, epsilon, maxValue,
                                     knap_data.get_value(), is_0_1_Tag);

    if (!multiplier) {
        return knapsack_check_integrality(std::move(knap_data), is_0_1_Tag,
                                          retrieve_solution);
    }

    auto newValue = utils::make_scale_functor<double, Value>(
        knap_data.get_value(), *multiplier);
    auto ret = knapsack_check_integrality(
        detail::make_knapsack_data(objects, knap_data.get_capacity(),
                                   knap_data.get_size(), newValue,
                                   knap_data.get_output_iter()),
        is_0_1_Tag, retrieve_solution);
    return std::make_pair(Value(double(ret.first) / *multiplier), ret.second);
}

template <typename KnapsackData, typename IsZeroOne, typename RetrieveSolution,
          typename ReturnType = typename KnapsackData::return_type>
ReturnType knapsack_general_on_size_fptas(double epsilon,
                                          KnapsackData knap_data,
                                          IsZeroOne is_0_1_Tag,
                                          RetrieveSolution retrieve_solution) {
    using ObjectRef = typename KnapsackData::object_ref;
    using Size = typename KnapsackData::size;

    auto &&objects = knap_data.get_objects();

    if (boost::empty(objects)) {
        return ReturnType{};
    }

    auto multiplier = get_multiplier(objects, epsilon, knap_data.get_capacity(),
                                     knap_data.get_size(), is_0_1_Tag);

    if (!multiplier) {
        return knapsack_check_integrality(std::move(knap_data), is_0_1_Tag,
                                          retrieve_solution);
    }

    auto newSize = utils::make_scale_functor<double, Size>(knap_data.get_size(),
                                                           *multiplier);
    auto ret = knapsack_check_integrality(
        detail::make_knapsack_data(
            objects, Size(knap_data.get_capacity() * *multiplier), newSize,
            knap_data.get_value(), knap_data.get_output_iter()),
        is_0_1_Tag, retrieve_solution);
    return ReturnType(ret.first, double(ret.second) / *multiplier);
}

template <typename KnapsackData, typename IsZeroOne>
typename KnapsackData::return_type
knapsack_general_on_value_fptas_retrieve(double epsilon, KnapsackData knap_data,
                                         IsZeroOne is_0_1_Tag) {
    using ObjectRef = typename KnapsackData::object_ref;
    using Value = typename KnapsackData::value;

    Value realValue{};
    auto addValue = [&](ObjectRef obj) {
        realValue += knap_data.get_value(obj);
        knap_data.out(obj);
    }
    ;

    auto newOut = boost::make_function_output_iterator(addValue);

    auto reducedReturn = knapsack_general_on_value_fptas(
        epsilon, detail::make_knapsack_data(
                     knap_data.get_objects(), knap_data.get_capacity(),
                     knap_data.get_size(), knap_data.get_value(), newOut),
        is_0_1_Tag, retrieve_solution_tag{});
    return std::make_pair(realValue, reducedReturn.second);
}

template <typename KnapsackData, typename IsZeroOne,
          typename ReturnType = typename KnapsackData::return_type>
ReturnType knapsack_general_on_size_fptas_retrieve(double epsilon,
                                                   KnapsackData knap_data,
                                                   IsZeroOne is_0_1_Tag) {
    using ObjectRef = typename KnapsackData::object_ref;
    using Size = typename KnapsackData::size;

    Size realSize{};
    auto add_size = [&](ObjectRef obj) {
        realSize += knap_data.get_size(obj);
        knap_data.out(obj);
    }
    ;

    auto newOut = boost::make_function_output_iterator(add_size);

    auto reducedReturn = knapsack_general_on_size_fptas(
        epsilon, detail::make_knapsack_data(
                     knap_data.get_objects(), knap_data.get_capacity(),
                     knap_data.get_size(), knap_data.get_value(), newOut),
        is_0_1_Tag, retrieve_solution_tag{});
    return ReturnType(reducedReturn.first, realSize);
}

} // detail
} // paal

#endif // PAAL_KNAPSACK_FPTAS_COMMON_HPP
