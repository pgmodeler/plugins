//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2014 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file euclidean_metric.hpp
 * @brief
 * @author Piotr Wygocki, Piotr Smulewicz
 * @version 1.0
 * @date 2013-10-28
 */
#ifndef PAAL_EUCLIDEAN_METRIC_HPP
#define PAAL_EUCLIDEAN_METRIC_HPP

#include "paal/data_structures/metric/metric_traits.hpp"
#include "paal/utils/type_functions.hpp"

#include <cmath>

namespace paal {
namespace data_structures {


/// metric with euclidean distance
template <typename T> struct euclidean_metric {
    /// operator()
    auto operator()(const std::pair<T, T> &p1, const std::pair<T, T> &p2) const
        -> decltype(std::hypot(p1.first - p2.first, p1.second - p2.second)) {
        return std::hypot(p1.first - p2.first, p1.second - p2.second);
    }
};

template <typename T>
struct metric_traits<euclidean_metric<T>>
    : public _metric_traits<euclidean_metric<T>, std::pair<T, T>> {};
} // data_structures

} // paal

#endif // PAAL_EUCLIDEAN_METRIC_HPP
