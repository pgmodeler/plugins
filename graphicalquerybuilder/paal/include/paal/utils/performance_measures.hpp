//=======================================================================
// Copyright (c) 2014 Andrzej Pacuk
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file performance_measures.hpp
 * @brief
 * @author Andrzej Pacuk
 * @version 1.0
 * @date 2014-10-22
 */
#ifndef PALL_PERFORMANCE_MEASURES_HPP
#define PALL_PERFORMANCE_MEASURES_HPP

#include <boost/range/combine.hpp>
#include <boost/range/empty.hpp>
#include <boost/range/size.hpp>
#include <boost/tuple/tuple.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace paal {

/**
 * @brief
 *
 * @tparam FloatType
 * @tparam Probs
 * @tparam TestResults
 *
 * @param probs
 * @param test_results
 */
template<typename FloatType = double,
         typename Probs, typename TestResults>
FloatType log_loss(Probs &&probs, TestResults &&test_results) {
    assert(boost::size(probs) == boost::size(test_results));
    assert(!boost::empty(probs));

    FloatType loss{};
    static FloatType EPSILON{1e-6};
    for(auto prob_result : boost::combine(probs, test_results)) {
        FloatType prob, result;
        boost::tie(prob, result) = prob_result;
        loss -= std::log(std::max(result ? prob : 1 - prob,
                                  EPSILON));
    }

    return loss / boost::size(probs);
}

/**
 * @brief
 *
 * @tparam FloatType
 * @param log_loss
 *
 * @return
 */
template<typename FloatType>
FloatType likelihood_from_log_loss(FloatType log_loss) {
    return std::exp(-log_loss);
}

/**
 * @brief
 *
 * @tparam FloatType
 * @tparam Probs
 * @tparam TestResults
 *
 * @param probs
 * @param test_results
 */
template<typename FloatType = double,
         typename Probs, typename TestResults>
FloatType likelihood(Probs &&probs, TestResults &&test_results) {
    return likelihood_from_log_loss(log_loss<FloatType>(std::forward<Probs>(probs),
                                    std::forward<TestResults>(test_results)));
}

/**
 * @brief
 *
 * @tparam FloatType
 * @tparam Probs
 * @tparam TestResults
 *
 * @param probs
 * @param test_results
 */
template<typename FloatType = double,
typename Probs, typename TestResults>
FloatType mean_absolute_error(Probs &&probs, TestResults &&test_results) {
    assert(boost::size(probs) == boost::size(test_results));
    assert(!boost::empty(probs));

    FloatType loss{};
    for(auto prob_result : boost::combine(probs, test_results)) {
        FloatType prob, result;
        boost::tie(prob, result) = prob_result;
        loss += std::abs(prob - result);
    }

    return loss / boost::size(probs);
}

} //! paal

#endif /* PALL_PERFORMANCE_MEASURES_HPP */

