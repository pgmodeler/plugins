//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file less_pointees.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-29
 */
#ifndef PAAL_LESS_POINTEES_HPP
#define PAAL_LESS_POINTEES_HPP
namespace paal {

// TODO add to boost
/**
 * @brief compare pointee using comparator
 *
 * @tparam Comparator
 */
template <class Comparator> struct less_pointees_t {
    /// constructor
    less_pointees_t(Comparator compare) : m_compare(compare) {}

    /// compare operator()
    template <typename OptionalPointee>
    bool operator()(OptionalPointee const &x, OptionalPointee const &y) const {
        return !y ? false : (!x ? true : m_compare(*x, *y));
    }

  private:
    Comparator m_compare;
};

/**
 * @brief  make function for less_pointees_t
 *
 * @tparam Comparator
 * @param compare
 *
 * @return
 */
template <class Comparator>
less_pointees_t<Comparator> make_less_pointees_t(Comparator compare) {
    return less_pointees_t<Comparator>(compare);
}

} //! paal
#endif // PAAL_LESS_POINTEES_HPP
