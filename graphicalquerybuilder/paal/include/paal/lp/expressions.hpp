//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file expressions.hpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2014-04-02
 */

#ifndef PAAL_EXPRESSIONS_HPP
#define PAAL_EXPRESSIONS_HPP

#include "paal/lp/ids.hpp"
#include "paal/utils/floating.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/print_collection.hpp"
#include "paal/utils/pretty_stream.hpp"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/count_if.hpp>

#include <functional>
#include <unordered_map>

namespace paal {
namespace lp {

namespace {
struct linear_expression_traits {
    static const utils::compare<double> CMP;
};

const utils::compare<double> linear_expression_traits::CMP = utils::compare<double>();
}

/**
 * Expression class.
 */
class linear_expression {
    typedef std::unordered_map<col_id, double> Elements;
    typedef Elements::const_iterator ExprIter;

  public:
    /// Constructor.
    linear_expression() {}

    /// Constructor.
    linear_expression(col_id col, double coef = 1.) {
        m_coefs.emplace(col, coef);
    }

    /// Addition operator.
    linear_expression &operator+=(const linear_expression &expr) {
        join_expression(expr, utils::plus{});
        return *this;
    }

    /// Subtraction operator.
    linear_expression &operator-=(const linear_expression &expr) {
        join_expression(expr, utils::minus{});
        return *this;
    }

    /// Multiplication by a constant operator.
    linear_expression &operator*=(double val) {
        for (auto &elem : m_coefs) {
            elem.second *= val;
        }
        return *this;
    }

    /// Division by a constant operator.
    linear_expression &operator/=(double val) { return operator*=(1. / val); }

    /// Returns the coefficient for a given column.
    double get_coefficient(col_id col) const {
        auto elem = m_coefs.find(col);
        if (elem != m_coefs.end()) {
            return elem->second;
        } else {
            return 0.;
        }
    }

    /// Returns the iterator range of the elements in the expression.
    const Elements &get_elements() const { return m_coefs; }

    /// Returns the size (number of nonzero coefficients) of the expression.
    int non_zeros() const {
        return boost::count_if(m_coefs, [](std::pair<col_id, double> x) {
            return !linear_expression_traits::CMP.e(x.second, 0);
        });
    }

  private:
    template <typename Operation>
    void join_expression(const linear_expression &expr, Operation op) {
        for (auto new_elem : expr.m_coefs) {
            auto elem = m_coefs.find(new_elem.first);
            if (elem == m_coefs.end()) {
                new_elem.second = op(0., new_elem.second);
                m_coefs.insert(new_elem);
            } else {
                elem->second = op(elem->second, new_elem.second);
            }
        }
    }

    Elements m_coefs;
};

namespace detail {
inline std::string col_id_to_string(col_id col) {
    return " x_" + std::to_string(col.get());
}

template <typename Stream, typename PrintCol>
void print_expression(Stream &o, const linear_expression &expr,
                      PrintCol print_col) {
    print_collection(o, expr.get_elements() |
                            boost::adaptors::transformed(
                                [&](std::pair<col_id, double> col_and_val) {
        return pretty_to_string(col_and_val.second) +
               print_col(col_and_val.first);
    }),
                     " + ");
}
};

/// operator<< : printing expression
template <typename Stream>
Stream &operator<<(Stream &o, const linear_expression &expr) {
    detail::print_expression(o, expr, detail::col_id_to_string);
    return o;
}

/// linear_expression + linear_expression operator.
inline linear_expression operator+(linear_expression expr_left,
                                   const linear_expression &expr_right) {
    expr_left += expr_right;
    return expr_left;
}

/// linear_expression - linear_expression operator.
inline linear_expression operator-(linear_expression expr_left,
                                   const linear_expression &expr_right) {
    expr_left -= expr_right;
    return expr_left;
}

/// linear_expression * double operator.
inline linear_expression operator*(linear_expression expr, double val) {
    expr *= val;
    return expr;
}

/// double * linear_expression operator.
inline linear_expression operator*(double val, const linear_expression &expr) {
    return expr * val;
}

/// linear_expression / double operator.
inline linear_expression operator/(linear_expression expr, double val) {
    expr /= val;
    return expr;
}

/// Unary - operator.
inline linear_expression operator-(const linear_expression &expr) {
    return expr * (-1.);
}

} // lp
} // paal

#endif // PAAL_EXPRESSIONS_HPP
