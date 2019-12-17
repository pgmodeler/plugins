//=======================================================================
// Copyright (c) 2014 Andrzej Pacuk
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file lsh_nearest_neighbors_regression_efficient_example.cpp
 * @brief
 * @author Andrzej Pacuk
 * @version 1.0
 * @date 2014-10-08
 */

//! [LSH Nearest Neighbors Regression Efficient Example]
#include "paal/regression/lsh_nearest_neighbors_regression.hpp"
#include "paal/utils/functors.hpp"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

int main() {
    using coordinate_t = int;
    using result_t = double;
    using point_coordinates_t = std::vector<coordinate_t>;
    using training_point_t = std::tuple<point_coordinates_t, result_t>;

    constexpr paal::utils::tuple_get<0> get_coordinates{};
    constexpr paal::utils::tuple_get<1> get_result{};

    const std::vector<training_point_t> training_points = {
            training_point_t{{0, 0}, 0.0}, training_point_t{{0, 1}, 0.4},
            training_point_t{{1, 0}, 0.6}, training_point_t{{1, 1}, 1.0}};
    const std::vector<point_coordinates_t> test_points =
            {{0, -1}, {2, 1}};

    auto const passes = 50;
    auto const dimensions = std::get<0>(training_points.front()).size();
    auto const hash_functions_per_point = 1;

    auto model = paal::make_lsh_nearest_neighbors_regression_tuple_hash(
            training_points | boost::adaptors::transformed(get_coordinates),
            training_points | boost::adaptors::transformed(get_result),
            passes,
            paal::lsh::hamming_hash_function_generator{dimensions},
            hash_functions_per_point);

    std::vector<result_t> results;
    results.reserve(test_points.size());
    model.test(test_points, std::back_inserter(results));

    std::cout << "Solution:" << std::endl;
    boost::copy(results, std::ostream_iterator<double>(std::cout, ","));
    std::cout << std::endl;

    return 0;
}
//! [LSH Nearest Neighbors Regression Efficient Example]

