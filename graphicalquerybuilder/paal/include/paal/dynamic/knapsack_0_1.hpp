//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_0_1.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-30
 */
#ifndef PAAL_KNAPSACK_0_1_HPP
#define PAAL_KNAPSACK_0_1_HPP

#include "paal/utils/functors.hpp"
#include "paal/utils/knapsack_utils.hpp"
#include "paal/utils/less_pointees.hpp"
#include "paal/utils/irange.hpp"
#include "paal/dynamic/knapsack/fill_knapsack_dynamic_table.hpp"
#include "paal/dynamic/knapsack/knapsack_common.hpp"
#include "paal/greedy/knapsack_0_1_two_app.hpp"

#include <boost/range/adaptor/reversed.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace paal {

namespace detail {

/**
 * @brief For 0/1 knapsack dynamic algorithm for given element the table has to
 * be traversed from the highest to the lowest element
 */
struct Knapsack_0_1_get_position_range {
    template <typename T>
    auto operator()(T begin, T end)
        ->decltype(irange(begin, end) | boost::adaptors::reversed) {
        return irange(begin, end) | boost::adaptors::reversed;
    }
};

/**
 * @brief This class helps solving 0/1 knapsack problem.
 *        Function solve returns the optimal value
 *        Function Retrieve solution returns chosen elements
 *
 * @tparam Objects
 * @tparam ObjectSizeFunctor
 * @tparam ObjectValueFunctor
 */
template <typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor, typename Comparator>
class Knapsack_0_1 {
    using base = knapsack_base<Objects, ObjectSizeFunctor, ObjectValueFunctor>;
    using SizeType = typename base::SizeType;
    using ValueType = typename base::ValueType;
    using ObjectType = typename base::ObjectType;
    using ObjectRef = typename base::ObjectRef;
    using ReturnType = typename base::return_type;
    using ValueOrNull = boost::optional<ValueType>;
    static_assert(std::is_integral<SizeType>::value,
                  "Size type must be integral");
    using ValueOrNullVector = std::vector<ValueOrNull>;

  public:

    Knapsack_0_1(ObjectSizeFunctor size, ObjectValueFunctor value,
                 Comparator compare = Comparator())
        : m_size(size), m_value(value), m_comparator(compare) {}

    /**
     * @brief  Function solves dynamic programming problem
     * @returns the optimal value
     */
    template <typename GetBestElement>
    ReturnType solve(Objects objects, SizeType capacity,
                     GetBestElement getBest) {
        m_object_on_size.resize(capacity + 1);
        fill_table(m_object_on_size, objects, capacity);
        auto maxValue = getBest(m_object_on_size.begin(),
                                m_object_on_size.end(), m_comparator);

        if (maxValue != m_object_on_size.end()) {
            return ReturnType(**maxValue, maxValue - m_object_on_size.begin());
        } else {
            return ReturnType(ValueType{}, SizeType{});
        }
    }

    //@brief here we find actual solution
    // that is, the chosen objects
    // this is done by simple divide and conquer strategy
    template <typename OutputIterator>
    void retrieve_solution(ValueType maxValue, SizeType size, Objects objects,
                           OutputIterator & out) const {
        m_object_on_size.resize(size + 1);
        m_object_on_size_rec.resize(size + 1);
        retrieve_solution_rec(maxValue, size, std::begin(objects),
                              std::end(objects), out);
    }

  private:
    template <typename OutputIterator, typename ObjectsIter>
    void retrieve_solution_rec(ValueType maxValue, SizeType capacity,
                               ObjectsIter oBegin, ObjectsIter oEnd,
                               OutputIterator & out) const {
        if (maxValue == ValueType()) {
            return;
        }

        auto objNr = std::distance(oBegin, oEnd);
        assert(objNr);

        // boundary case only one object left
        if (objNr == 1) {
            assert(m_value(*oBegin) == maxValue);
            *out = *oBegin;
            ++out;
            return;
        }

        // main case, at least 2 objects left
        auto midle = oBegin + objNr / 2;
        fill_table(m_object_on_size, boost::make_iterator_range(oBegin, midle),
                   capacity);
        fill_table(m_object_on_size_rec,
                   boost::make_iterator_range(midle, oEnd), capacity);

        SizeType capacityLeftPartInOptimalSolution{};
        for (auto capacityLeftPart : irange(capacity + 1)) {
            auto left = m_object_on_size[capacityLeftPart];
            auto right = m_object_on_size_rec[capacity - capacityLeftPart];
            if (left && right) {
                if (*left + *right == maxValue) {
                    capacityLeftPartInOptimalSolution = capacityLeftPart;
                    break;
                }
            }
        }
        auto left = m_object_on_size[capacityLeftPartInOptimalSolution];
        auto right =
            m_object_on_size_rec[capacity - capacityLeftPartInOptimalSolution];
        assert(left && right && *left + *right == maxValue);

        retrieve_solution_rec(*left, capacityLeftPartInOptimalSolution, oBegin,
                              midle, out);
        retrieve_solution_rec(
            *right, capacity - capacityLeftPartInOptimalSolution, midle, oEnd,
            out);
    }

    template <typename ObjectsRange>
    void fill_table(ValueOrNullVector &values, ObjectsRange &&objects,
                    SizeType capacity) const {
        fill_knapsack_dynamic_table(
            values.begin(), values.begin() + capacity + 1,
            std::forward<ObjectsRange>(objects), m_size,
            [&](ValueOrNull val,
                typename boost::range_iterator<ObjectsRange>::type obj) {
            return *val + m_value(*obj);
        },
            [&](ValueOrNull left, ValueOrNull right) {
            return m_comparator(*left, *right);
        },
            [](ValueOrNull & val) {
            val = ValueType{};
        },
            Knapsack_0_1_get_position_range{});
    }

    ObjectSizeFunctor m_size;
    ObjectValueFunctor m_value;
    Comparator m_comparator;
    mutable ValueOrNullVector m_object_on_size;
    mutable ValueOrNullVector m_object_on_size_rec;
};

template <typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor, typename Comparator>
Knapsack_0_1<Objects, ObjectSizeFunctor, ObjectValueFunctor, Comparator>
make_knapsack_0_1(ObjectSizeFunctor size, ObjectValueFunctor value,
                  Comparator comp) {
    return Knapsack_0_1<Objects, ObjectSizeFunctor, ObjectValueFunctor,
                        Comparator>(size, value, comp);
}

template <typename Knapsack, typename IndexType, typename ValueType,
          typename Objects, typename OutputIterator>
void retrieve_solution(const Knapsack &knapsack, ValueType maxValue,
                       IndexType size, Objects &&objects, OutputIterator & out,
                       retrieve_solution_tag) {
    knapsack.retrieve_solution(maxValue, size, objects, out);
}

template <typename Knapsack, typename IndexType, typename ValueType,
          typename Objects, typename OutputIterator>
void retrieve_solution(const Knapsack &knapsack, ValueType maxValue,
                       IndexType size, Objects &&objects, OutputIterator & out,
                       no_retrieve_solution_tag) {}

/**
 * @brief Solution to Knapsack 0/1 problem
 *  overload for integral Size case
 */
template <typename KnapsackData, typename retrieve_solution_tag>
auto knapsack(KnapsackData knap_data, zero_one_tag, integral_size_tag,
         retrieve_solution_tag retrieve_solutionTag) {
    using Value = typename KnapsackData::value;

    auto knapsack = make_knapsack_0_1<typename KnapsackData::objects>(
        knap_data.get_size(), knap_data.get_value(), utils::less{});
    auto value_size =
        knapsack.solve(knap_data.get_objects(), knap_data.get_capacity(),
                       get_max_element_on_capacity_indexed_collection<Value>());
    retrieve_solution(knapsack, value_size.first, value_size.second,
                      knap_data.get_objects(), knap_data.get_output_iter(),
                      retrieve_solutionTag);
    return value_size;
}

/**
 * @brief Solution to Knapsack 0/1 problem
 *  overload for integral Value case
 */
template <typename KnapsackData, typename retrieve_solution_tag>
auto knapsack(KnapsackData knap_data, zero_one_tag, integral_value_tag,
         retrieve_solution_tag retrieve_solutionTag) {
    using Value = typename KnapsackData::value;
    using Size = typename KnapsackData::size;

    auto knapsack = make_knapsack_0_1<typename KnapsackData::objects>(
        knap_data.get_value(), knap_data.get_size(), utils::greater{});
    auto maxValue = get_value_bound(knap_data, zero_one_tag{}, upper_tag{});
    auto value_size = knapsack.solve(
        knap_data.get_objects(), maxValue,
        get_max_element_on_value_indexed_collection<boost::optional<Size>,
                                                    Value>(
            boost::optional<Size>(knap_data.get_capacity() + 1)));
    retrieve_solution(knapsack, value_size.first, value_size.second,
                      knap_data.get_objects(), knap_data.get_output_iter(),
                      retrieve_solutionTag);
    return std::make_pair(value_size.second, value_size.first);
}

} // detail

/**
 * @brief Solution to Knapsack 0/1 problem
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
template <typename Objects, typename OutputIterator, typename ObjectSizeFunctor,
          typename ObjectValueFunctor = utils::return_one_functor>
auto knapsack_0_1(Objects &&objects,
             detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
                 capacity, // capacity is of size type
             OutputIterator out, ObjectSizeFunctor size,
             ObjectValueFunctor value = ObjectValueFunctor{}) {

    return detail::knapsack_check_integrality(
        detail::make_knapsack_data(std::forward<Objects>(objects), capacity,
                                   size, value, out),
        detail::zero_one_tag{});
}

/**
 * @brief Solution to Knapsack 0/1 problem, without retrieving the objects in
* the solution
 *
 * @tparam Objects
 * @tparam OutputIterator
 * @tparam ObjectSizeFunctor
 * @tparam ObjectValueFunctor
 * @param oBegin given objects
 * @param oEnd
 * @param size functor that for given object returns its size
 * @param value functor that for given object returns its value
 */
template <typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor = utils::return_one_functor>
auto knapsack_0_1_no_output(Objects &&objects,
                       detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
                           capacity, // capacity is of size type
                       ObjectSizeFunctor size,
                       ObjectValueFunctor value = ObjectValueFunctor{}) {
    auto out = boost::make_function_output_iterator(utils::skip_functor{});
    return detail::knapsack_check_integrality(
        detail::make_knapsack_data(
            std::forward<Objects>(objects), capacity, size, value, out),
        detail::zero_one_tag{}, detail::no_retrieve_solution_tag{});
}

} // paal

#endif // PAAL_KNAPSACK_0_1_HPP
