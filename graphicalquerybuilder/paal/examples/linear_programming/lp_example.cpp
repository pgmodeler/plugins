//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file lp_example.cpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2014-03-31
 */

//! [LP Example]
#include "paal/lp/glp.hpp"

#include <iostream>

using LP = paal::lp::glp;

void print_solution(paal::lp::problem_type status, const LP &lp_instance) {
    std::cout << "-----------------------------------------" << std::endl;
    if (status == paal::lp::OPTIMAL) {
        std::cout << "Optimal solution cost: " << lp_instance.get_obj_value()
                  << std::endl;
    } else {
        std::cout << "Optimal solution not found" << std::endl;
    }
    std::cout << lp_instance << std::endl;
    // TODO example should show how to read a valuation of a variable, one
    // has to search documentation quite extensively to find that
}

int main() {
    // sample problem
    LP lp_instance;

    lp_instance.set_optimization_type(paal::lp::MAXIMIZE);
    // add_column(column_cost, lower_bound, upper_bound, symbolic_name)
    auto X = lp_instance.add_column(500, 0, paal::lp::lp_traits::PLUS_INF, "x");
    auto Y = lp_instance.add_column(300, 0, paal::lp::lp_traits::PLUS_INF, "y");

    // TODO example should should show how to create an expression with
    // 0 or 1 variable. This is the most standard use case, we use that
    // every time we create an expression in the loop
    auto expr = X + Y;
    std::cout << expr << std::endl;
    std::cout << (expr >= 7) << std::endl;
    lp_instance.add_row(expr >= 7);
    auto row = lp_instance.add_row(expr <= 10);
    lp_instance.add_row(15 <= 200 * X + 100 * Y <= 1200);

    // solve the LP
    auto status = lp_instance.solve_simplex();
    print_solution(status, lp_instance);

    // add new row
    expr += Y;
    lp_instance.add_row(expr == 12);
    // modify row bound
    lp_instance.set_row_upper_bound(row, 20);
    // modify column bound
    lp_instance.set_col_lower_bound(X, -50);
    // modify row expression
    lp_instance.set_row_expression(row, X + Y * 1.01);

    // resolve the LP
    status = lp_instance.resolve_simplex(paal::lp::DUAL);
    print_solution(status, lp_instance);

    // delete row
    lp_instance.delete_row(row);

    // resolve the LP
    status = lp_instance.resolve_simplex();
    print_solution(status, lp_instance);

    return 0;
}
//! [LP Example]
