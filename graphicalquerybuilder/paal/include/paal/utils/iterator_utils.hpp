//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file iterator_utils.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
#include "type_functions.hpp"
#include "functors.hpp"

#include <boost/iterator/filter_iterator.hpp>

#include <functional>

#ifndef PAAL_ITERATOR_UTILS_HPP
#define PAAL_ITERATOR_UTILS_HPP

namespace paal {
namespace utils {

/**
 * @brief this iterator exlcludes one specific element from range
 *
 * @tparam Iterator
 */
template <typename Iterator>
struct iterator_with_excluded_element : public boost::filter_iterator<
    decltype(std::bind(
        utils::not_equal_to(),
        std::declval<typename std::iterator_traits<Iterator>::value_type>(),
        std::placeholders::_1)),
    Iterator> {

    typedef typename std::iterator_traits<Iterator>::value_type Element;
    /**
     * @brief constructor
     *
     * @param i
     * @param end
     * @param e
     */
    iterator_with_excluded_element(Iterator i, Iterator end, const Element &e)
        : boost::filter_iterator<
              decltype(std::bind(utils::not_equal_to(), std::declval<Element>(),
                                 std::placeholders::_1)),
              Iterator>(std::bind(utils::not_equal_to(), e,
                                  std::placeholders::_1),
                        i, end) {}

    iterator_with_excluded_element() = default;
};

} // utils
} // paal

#endif // PAAL_ITERATOR_UTILS_HPP
