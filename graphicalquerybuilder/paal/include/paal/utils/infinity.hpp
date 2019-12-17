//=======================================================================
// Copyright (c) 2014 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file infinity.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-04-29
 */
#ifndef PAAL_INFINITY_HPP
#define PAAL_INFINITY_HPP

namespace paal {

namespace detail {

/**
 * @brief if the sign = true, class represents plus_infinity: object bigger than
 * everything
 *        if the sign = false, class represents minus_infinity
 */
template <bool sign> class infinity {
    // this is used to disable some operators overloads to remove disambiguities
    template <typename T>
    using disable_other_infinity = typename std::enable_if<!std::is_same<
        typename std::decay<T>::type, infinity<!sign>>::value>::type;

  public:
    /// operator<
    template <typename T> bool operator<(T &&) const { return !sign; }

    /// operator<
    bool operator<(infinity) const { return false; }

    /// operator>
    template <typename T> bool operator>(T &&) const { return sign; }

    /// operator>
    bool operator>(infinity) const { return false; }

    /// operator<=
    template <typename T> bool operator<=(T &&t) const {
        return !(*this > std::forward<T>(t));
    }

    /// operator<=
    bool operator<=(infinity) const { return true; }

    /// operator>=
    template <typename T> bool operator>=(T &&t) const {
        return !(*this < std::forward<T>(t));
    }

    /// operator>=
    bool operator>=(infinity) const { return true; }

    /// friend operator<
    template <typename T, typename = disable_other_infinity<T>>
    friend bool operator<(T &&t, infinity) {
        return infinity{}
        > std::forward<T>(t);
    }

    /// friend operator>
    template <typename T, typename = disable_other_infinity<T>>
    friend bool operator>(T &&t, infinity) {
        return infinity{}
        < std::forward<T>(t);
    }

    /// friend operator<=
    template <typename T, typename = disable_other_infinity<T>>
    friend bool operator<=(T &&t, infinity) {
        return infinity{}
        >= std::forward<T>(t);
    }

    /// friend operator>=
    template <typename T, typename = disable_other_infinity<T>>
    friend bool operator>=(T &&t, infinity) {
        return infinity{}
        <= std::forward<T>(t);
    }
};

template <bool sign> bool operator<(infinity<!sign>, infinity<sign>) {
    return sign;
}

template <bool sign> bool operator>(infinity<sign>, infinity<!sign>) {
    return sign;
}

template <bool sign>
bool operator<=(infinity<!sign> left, infinity<sign> right) {
    return !(left > right);
}

template <bool sign>
bool operator>=(infinity<!sign> left, infinity<sign> right) {
    return !(left < right);
}

} //!detail

using minus_infinity = detail::infinity<false>;
using plus_infinity = detail::infinity<true>;

} //!paal

#endif // PAAL_INFINITY_HPP
