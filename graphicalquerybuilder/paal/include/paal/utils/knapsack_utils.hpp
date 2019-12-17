//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file knapsack_utils.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-07
 */
#ifndef PAAL_KNAPSACK_UTILS_HPP
#define PAAL_KNAPSACK_UTILS_HPP

#include "paal/utils/type_functions.hpp"
#include "paal/utils/less_pointees.hpp"
#include "paal/utils/functors.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/numeric.hpp>

namespace paal {

/**
 * @brief density functor, for given value and size
 *
 * @tparam Value
 * @tparam Size
 */
template <typename Value, typename Size> struct density {

    /// constructor
    density(Value value, Size size) : m_value(value), m_size(size) {}

    /// operator()
    template <typename ObjectRef> double operator()(ObjectRef obj) const {
        return double(m_value(obj)) / double(m_size(obj));
    }

  private:
    Value m_value;
    Size m_size;
};

/**
 * @brief make for density
 *
 * @tparam Value
 * @tparam Size
 * @param value
 * @param size
 *
 * @return
 */
template <typename Value, typename Size>
density<Value, Size> make_density(Value value, Size size) {
    return density<Value, Size>(value, size);
}


namespace detail {
template <typename Functor, typename Range>
using FunctorOnRangePValue =
    puretype(std::declval<Functor>()(*std::begin(std::declval<Range>())));

// definition of basic types
template <typename Objects, typename ObjectSizeFunctor,
          typename ObjectValueFunctor>
struct knapsack_base {
    typedef detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects> SizeType;
    typedef detail::FunctorOnRangePValue<ObjectValueFunctor, Objects> ValueType;
    typedef puretype(*std::begin(std::declval<Objects>())) ObjectType;
    using PureObjRef = typename boost::range_reference<Objects>::type;
    // add_const is unfortunately needed by function_output_iterator
    // TODO it supposed to be changed in boost...
    typedef typename std::conditional<
        std::is_reference<PureObjRef>::value,
        typename std::add_lvalue_reference<typename std::add_const<
            typename std::remove_reference<PureObjRef>::type>::type>::type,
        typename std::add_const<PureObjRef>::type>::type ObjectRef;
    typedef std::pair<ValueType, SizeType> return_type;
};


// various tags
struct integral_value_and_size_tag {};
struct integral_value_tag {};
struct integral_size_tag {};
struct non_integral_value_and_size_tag {};

struct arithmetic_size_tag {};
struct Nonarithmetic_size_tag {};

struct zero_one_tag {};
struct unbounded_tag {};

struct retrieve_solution_tag {};
struct no_retrieve_solution_tag {};

template <typename GetSize, typename GetValue, typename Objects,
          typename OutputIterator>
class knapsack_data {
  public:
    using traits = knapsack_base<Objects, GetSize, GetValue>;
    using size = typename traits::SizeType;
    using value = typename traits::ValueType;
    using objects = Objects;
    using object_ref = typename traits::ObjectRef;
    using object_iter = typename boost::range_iterator<Objects>::type;
    using return_type = typename traits::return_type;

    knapsack_data(Objects objects, size capacity, GetSize get_size,
                  GetValue get_value, OutputIterator & out)
        : m_objects(objects), m_capacity(capacity), m_get_size(get_size),
          m_get_value(get_value), m_out(out) {}

    size get_size(object_ref obj) const { return m_get_size(obj); }

    value get_value(object_ref obj) const { return m_get_value(obj); }

    GetSize get_size() const { return m_get_size; }

    GetValue get_value() const { return m_get_value; }

    objects get_objects() { return m_objects; }

    void out(object_ref obj) {
        *m_out = obj;
        ++m_out;
    }

    size get_capacity() const { return m_capacity; }

    OutputIterator & get_output_iter() const { return m_out; }

    density<GetValue, GetSize> get_density() {
        return make_density(m_get_value, m_get_size);
    }


  private:
    Objects m_objects;
    size m_capacity;
    GetSize m_get_size;
    GetValue m_get_value;
    OutputIterator & m_out;
};

template <typename GetSize, typename GetValue, typename Objects,
          typename OutputIterator,
          typename Size = FunctorOnRangePValue<GetSize, Objects>>
knapsack_data<GetSize, GetValue, Objects, OutputIterator>
make_knapsack_data(Objects &&objects, Size capacity, GetSize get_size,
                   GetValue get_value, OutputIterator & out) {
    return knapsack_data<GetSize, GetValue, Objects, OutputIterator>{
        std::forward<Objects>(objects), capacity, get_size, get_value, out
    };
}


} //!detail

}      //! paal
#endif // PAAL_KNAPSACK_UTILS_HPP
