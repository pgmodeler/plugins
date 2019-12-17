//=======================================================================
// Copyright (c) 2014 Andrzej Pacuk
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file accumulate_functors.hpp
 * @brief
 * @author Piotr Wygocki, Robert Rosolek, Andrzej Pacuk
 * @version 1.0
 * @date 2014-11-18
 */
#ifndef PAAL_ACCUMULATE_FUNCTORS_HPP
#define PAAL_ACCUMULATE_FUNCTORS_HPP

#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/numeric.hpp>

namespace paal {

/// combination of boost::accumulate and boost::adaptors::transformed
template <
    typename Range,
    typename T,
    typename Functor,
    typename BinaryOperation = utils::plus>
T accumulate_functor(
    const Range& rng,
    T init,
    Functor f,
    BinaryOperation bin_op = BinaryOperation{}) {
    return boost::accumulate(
        rng | boost::adaptors::transformed(f),
        init,
        bin_op);
}

/// sum of functor values over the range elements
template <typename Range, typename Functor>
auto sum_functor(const Range& rng, Functor f) {
    using T = pure_result_of_t<Functor(range_to_elem_t<Range>)>;
    return accumulate_functor(rng, T{}, f);
}

/// combination of boost::max_element and boost::adaptors::transformed
template <typename Range, typename Functor>
auto max_element_functor(Range&& range, Functor f) {
    auto unsafe_to_return_iterator = boost::max_element(
            std::forward<Range>(range) |
            boost::adaptors::transformed(utils::make_assignable_functor(f)));
    return boost::make_transform_iterator(unsafe_to_return_iterator.base(), f);
}

/// combination of boost::min_element and boost::adaptors::transformed
template <typename Range, typename Functor>
auto min_element_functor(Range&& range, Functor f) {
    auto unsafe_to_return_iterator = boost::min_element(
            std::forward<Range>(range) |
            boost::adaptors::transformed(utils::make_assignable_functor(f)));
    return boost::make_transform_iterator(unsafe_to_return_iterator.base(), f);
}

/// helper class facilitating counting average
template <typename ValueType = double, typename CounterType = std::size_t>
class average_accumulator {
    ValueType m_accumulated_value;
    CounterType m_cnt;

public:
    ///serialize
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_accumulated_value;
        ar & m_cnt;
    }

    /**
     * @brief
     *
     * @param value
     * @param cnt
     */
    average_accumulator(ValueType value = ValueType{},
                        CounterType cnt = CounterType{}) :
        m_accumulated_value(value), m_cnt(cnt) {
    }

    ///operator==
    bool operator==(average_accumulator other) const {
        return m_accumulated_value == other.m_accumulated_value &&
               m_cnt == other.m_cnt;
    }

    /**
     * @brief
     *
     * @param value
     * @param cnt
     */
    void add_value(ValueType value, CounterType cnt = 1) {
        m_accumulated_value += value;
        m_cnt += cnt;
    }

    /**
     * @brief
     *
     * @param accumulator
     */
    average_accumulator& operator+=(const average_accumulator &accumulator) {
        add_value(accumulator.m_accumulated_value, accumulator.m_cnt);
        return *this;
    }

    /**
     * @brief
     *
     * @return
     */
    ValueType get_accumulated_value() const {
        return m_accumulated_value;
    }

    /**
     * @brief
     *
     * @return
     */
    CounterType get_count() const {
        return m_cnt;
    }

    /**
     * @brief
     *
     * @return
     */
    bool empty() const {
        return m_cnt == 0;
    }

    /**
     * @brief
     *
     * @return
     */
    template <typename ReturnType = double>
    ReturnType get_average_unsafe() const {
        return static_cast<ReturnType>(m_accumulated_value) / m_cnt;
    }

    /**
     * @brief
     *
     * @param default_value
     *
     * @return
     */
    template <typename ReturnType = double>
    ReturnType get_average(ReturnType default_value = ReturnType{}) const {
        if (m_cnt) {
            return get_average_unsafe();
        } else {
            return default_value;
        }
    }

};

} //! paal
#endif /* PAAL_ACCUMULATE_FUNCTORS_HPP */
