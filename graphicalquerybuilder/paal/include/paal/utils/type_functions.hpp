//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file type_functions.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
#ifndef PAAL_TYPE_FUNCTIONS_HPP
#define PAAL_TYPE_FUNCTIONS_HPP

#include <boost/range.hpp>

#include <type_traits>
#include <utility>
#include <tuple>

namespace paal {

//TODO, remove when appears in the standard
/// short version of std::decay
template <typename T>
using decay_t = typename std::decay<T>::type;

/// for given expression returns its type with removed const and reference
#define puretype(t) typename std::decay<decltype(t)>::type

/// for given range returns type of its reference
template <typename Range>
using range_to_ref_t = typename boost::range_reference<Range>::type;

/// for given range returns type of its element
template <typename Range>
using range_to_elem_t = typename boost::range_value<Range>::type;

/// for given collection returns its difference type
template <typename Range>
using range_to_diff_type_t = typename boost::range_difference<Range>::type;

namespace detail {

    template <typename T, int k>
    struct k_tuple {
        using type = decltype(
            std::tuple_cat(std::declval<std::tuple<T>>(),
                       std::declval<typename k_tuple<T, k - 1>::type>()));
    };

    template <typename T>
    struct k_tuple<T, 1> {using type = std::tuple<T>;};
}

/// returns tuple consisting of k times type T
template <typename T, int k>
using k_tuple_t = typename detail::k_tuple<T, k>::type;

/// return pure type of function (decays const and reference)
template <class F>
using pure_result_of_t = typename std::decay<typename std::result_of<F>::type>::type;

/// return type obtained after adding values of given types
template <typename T1, typename T2>
using promote_with_t = puretype(std::declval<T1>() + std::declval<T2>());

/// return type after promotion with double
template <typename T>
using promote_with_double_t = promote_with_t<T, double>;

} //!paal

#endif // PAAL_TYPE_FUNCTIONS_HPP
