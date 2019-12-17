//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file problem_type.hpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2013-10-28
 */
#ifndef PAAL_PROBLEM_TYPE_HPP
#define PAAL_PROBLEM_TYPE_HPP

namespace paal {
namespace lp {
/// LP problem type.
enum problem_type {
    OPTIMAL,
    INFEASIBLE,
    UNBOUNDED,
    UNDEFINED
};
}
}
#endif // PAAL_PROBLEM_TYPE_HPP
