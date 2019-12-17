//=======================================================================
// Copyright (c) 2014
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file frequent_directions_example.cpp
 * @brief
 * @author Tomasz Strozak
 * @version 1.0
 * @date 2014-11-31
 */

//! [Frequent Directions Example]
#include "paal/sketch/frequent_directions.hpp"
#include "paal/utils/irange.hpp"
#include "paal/utils/print_collection.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <iostream>
#include <vector>

using coordinate_t = double;
using matrix_t = boost::numeric::ublas::matrix<coordinate_t>;

int main() {
    std::size_t const rows_count = 5;
    std::size_t const columns_count = 3;
    std::size_t const sketch_size = 4;

    auto fd_sketch = paal::make_frequent_directions<coordinate_t>(sketch_size, columns_count);

    matrix_t data(rows_count, columns_count);
    for (auto i : paal::irange(rows_count)) {
        for (auto j : paal::irange(columns_count)) {
            data(i, j) = i + j;
        }
    }
    fd_sketch.update(std::move(data));

    std::vector<std::vector<coordinate_t>> rows = {
        {2, 1, 0},
        {3, 2, 1},
        {4, 3, 2},
        {5, 4, 3},
        {6, 5, 4}};
    fd_sketch.update_range(std::move(rows));

    auto row = {7.0, 6.0, 5.0};
    fd_sketch.update_row(std::move(row));

    fd_sketch.compress();

    auto actual_size = fd_sketch.get_sketch().second;
    std::cout << "Actual sketch size: " << actual_size << std::endl;
    auto sketch = fd_sketch.get_sketch().first;
    boost::numeric::ublas::matrix_range<matrix_t> sketch_range (sketch,
         boost::numeric::ublas::range(0, actual_size),
         boost::numeric::ublas::range(0, columns_count));
    std::cout << "Sketch data:" << std::endl;
    paal::print_matrix(std::cout, sketch_range, " ");
    std::cout << std::endl;

    return 0;
}

//! [Frequent Directions Example]
