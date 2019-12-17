//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file fraction.hpp
 * @brief Implementation of fractions, which are used only for comparison purposes,
 * and thus they can be used with floating point types as well.
 * @author Robert Rosolek
 * @version 1.0
 * @date 2013-06-06
 */

#ifndef PAAL_FRACTION_HPP
#define PAAL_FRACTION_HPP

#include "paal/utils/floating.hpp"

namespace paal {
namespace data_structures {

/**
 * @brief simple class to represent fraction
 *
 * @tparam A
 * @tparam B
 */
template <class A, class B> struct fraction {
    ///numerator type
    using num_type = A;
    /// denominator type
    using den_type = B;
    /// numerator
    A num;
    /// denominator
    B den;
    /// constructor
    fraction(A num, B den) : num(num), den(den) {}
};

/**
 * @brief operator<
 *
 * @tparam A
 * @tparam B
 * @tparam C
 * @tparam D
 * @param f1
 * @param f2
 *
 * @return
 */
template <class A, class B, class C, class D>
bool operator<(const fraction<A, B> &f1, const fraction<C, D> &f2)
{
    return f1.num * f2.den < f2.num * f1.den;
}

/**
 * @brief operator>
 *
 * @tparam A
 * @tparam B
 * @tparam C
 * @tparam D
 * @param f1
 * @param f2
 *
 * @return
 */
template <class A, class B, class C, class D>
bool operator>(const fraction<A, B> &f1, const fraction<C, D> &f2)
{
    return f2 < f1;
}

/**
 * @brief operator<=
 *
 * @tparam A
 * @tparam B
 * @tparam C
 * @tparam D
 * @param f1
 * @param f2
 *
 * @return
 */
template <class A, class B, class C, class D>
bool operator<=(const fraction<A, B> &f1, const fraction<C, D> &f2)
{
    return !(f2 < f1);
}

/**
 * @brief operator>=
 *
 * @tparam A
 * @tparam B
 * @tparam C
 * @tparam D
 * @param f1
 * @param f2
 *
 * @return
 */
template <class A, class B, class C, class D>
bool operator>=(const fraction<A, B> &f1, const fraction<C, D> &f2)
{
    return !(f1 < f2);
}

/**
 * @brief operator==
 *
 * @tparam A
 * @tparam B
 * @tparam C
 * @tparam D
 * @tparam EPS
 * @param f1
 * @param f2
 * @param eps
 *
 * @return
 */
template<class A, class B, class C, class D, class EPS = A>
bool are_fractions_equal(const fraction<A, B>& f1, const fraction<C, D>& f2, EPS eps = A{})
{
    auto x = f1.num * f2.den - f2.num * f1.den;
    utils::compare<decltype(x)> cmp(eps);
    return cmp.e(x, 0);
}

/**
 * @brief make function for fraction
 *
 * @tparam A
 * @tparam B
 * @param a
 * @param b
 *
 * @return
 */
template <class A, class B>
fraction<A, B> make_fraction(A a, B b)
{
    return fraction<A, B>(a, b);
}

/**
 * @brief operator*
 *
 * @tparam A
 * @tparam B
 * @tparam C
 * @param c
 * @param f
 *
 * @return
 */
template<class A, class B, class C>
auto operator*(C c, const fraction<A, B>& f) {

    return make_fraction(c * f.num, f.den);
}

} //!data_structures
} //!paal

#endif // PAAL_FRACTION_HPP
