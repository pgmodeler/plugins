//=======================================================================
// Copyright (c) 2013 Piotr Wygocki, Maciej Andrejczuk
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file floating.hpp
 * @author Maciej Andrejczuk, Piotr Wygocki
 * @version 1.0
 * @date 2013-09-01
 */
#ifndef PAAL_FLOATING_HPP
#define PAAL_FLOATING_HPP

#include <limits>
#include <cmath>
#include <random>

namespace paal {
namespace utils {

///Class for comparing floating point
template <typename T>
class compare {
    T const m_epsilon;
public:
    ///constructor
    compare(T epsilon = std::numeric_limits<T>::epsilon()): m_epsilon(epsilon) {}

    ///equals
    bool e(T a, T b) const {
        return std::abs(a - b) < m_epsilon;
        // return abs(a -b ) < m_epsilon; //this line breaks
        // generalised_assignment_long_test TODO investigate
    }

    /// greater
    bool g(T a, T b) const { return a > b + m_epsilon; }

    /// greater equals
    bool ge(T a, T b) const { return a >= b - m_epsilon; }

    /// less equals
    bool le(T a, T b) const { return a <= b + m_epsilon; }

    /// get_epsilon used in comparison
    double get_epsilon() const { return m_epsilon; }


    /// returns default epsilon equals the smallest possible difference on doubles
    static T default_epsilon() { return std::numeric_limits<T>::epsilon(); }
};

} // utils
} // paal

#endif // PAAL_FLOATING_HPP
