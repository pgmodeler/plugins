/**
 * @file irange.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-10-3
 */
#ifndef PAAL_IRANGE_HPP
#define PAAL_IRANGE_HPP

#include <boost/range/irange.hpp>

namespace paal {

/**
 * @brief irange
 * @tparam T
 * @param begin
 * @param end
 */
template <typename T>
auto irange(T begin, T end) {
    return boost::irange(begin, end);
}

/**
 * @brief irange
 * @tparam T
 * @param end
 */
template <typename T>
auto irange(T end) {
    return irange(T{}, end);
}

}

#endif // PAAL_IRANGE_HPP
