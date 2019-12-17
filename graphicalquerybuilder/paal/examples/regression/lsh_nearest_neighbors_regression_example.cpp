//=======================================================================
// Copyright (c) 2014 Andrzej Pacuk
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file lsh_nearest_neighbors_regression_example.cpp
 * @brief
 * @author Andrzej Pacuk
 * @version 1.0
 * @date 2014-10-06
 */

//! [LSH Nearest Neighbors Regression Example]
#include "paal/regression/lsh_nearest_neighbors_regression.hpp"

#include <boost/numeric/ublas/vector.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <iostream>
#include <iterator>
#include <vector>

using coordinate_t = int;
using point_t = boost::numeric::ublas::vector<coordinate_t>;

auto make_point(const std::initializer_list<int> &list) {
    point_t point(list.size());
    boost::copy(list, point.begin());
    return point;
};

int main() {
    const std::vector<point_t> training_points =
            {make_point({0, 0}), make_point({0, 1}),
             make_point({1, 0}), make_point({1, 1})};
    const std::vector<double> training_results = {0.0, 0.4, 0.6, 1.0};
    const std::vector<point_t> test_points =
            {make_point({0, -1}), make_point({2, 1})};

    auto const passes = 50;
    auto const hash_functions_per_point = 10;
    auto const dimensions = training_points.front().size();
    auto const threads_count = 1;
    //w_param should be essentially bigger than radius of expected test point neighborhood
    auto const w_param = 3.0;

    auto lsh_function_generator =
        paal::lsh::l_2_hash_function_generator<>{dimensions, w_param};
    auto model = paal::make_lsh_nearest_neighbors_regression_tuple_hash(
                    training_points, training_results,
                    passes,
                    std::move(lsh_function_generator),
                    hash_functions_per_point,
                    threads_count);

    std::vector<double> results;
    model.test(test_points, std::back_inserter(results));

    std::cout << "Solution:" << std::endl;
    boost::copy(results, std::ostream_iterator<double>(std::cout, ","));
    std::cout << std::endl;

    return 0;
}
//! [LSH Nearest Neighbors Regression Example]

