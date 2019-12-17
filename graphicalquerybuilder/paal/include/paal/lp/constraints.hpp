//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file constraints.hpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2014-04-02
 */

#ifndef PAAL_CONSTRAINTS_HPP
#define PAAL_CONSTRAINTS_HPP

#include "paal/lp/expressions.hpp"

#include <type_traits>

namespace paal {
namespace lp {

// bound types
struct LowerBoundTag {};
struct UpperBoundTag {};

// bound directions
struct RightBoundTag {};
struct LeftBoundTag {};

namespace {
struct lp_traits {
    static const double PLUS_INF;
    static const double MINUS_INF;
};

const double lp_traits::PLUS_INF = std::numeric_limits<double>::infinity();
const double lp_traits::MINUS_INF = -std::numeric_limits<double>::infinity();
}

/**
 * Single bounded expression class.
 */
template <typename BoundTypeTag, typename BoundDirectionTag>
class single_bounded_expression {
  public:
    /// Constructor from expression and lower bound.
    single_bounded_expression(double lb, linear_expression expr)
        : m_lb(lb), m_expr(std::move(expr)) {
        assert_bound_type_tag<LowerBoundTag>();
    }

    /// Constructor from expression and upper bound.
    single_bounded_expression(linear_expression expr, double ub)
        : m_ub(ub), m_expr(std::move(expr)) {
        assert_bound_type_tag<UpperBoundTag>();
    }

    /// Expression getter.
    linear_expression get_expression() const { return m_expr; }

    /// Lower bound getter.
    double get_lower_bound() const {
        assert_bound_type_tag<LowerBoundTag>();
        return m_lb;
    }

    /// Upper bound getter.
    double get_upper_bound() const {
        assert_bound_type_tag<UpperBoundTag>();
        return m_ub;
    }

  private:
    static_assert(std::is_same<BoundDirectionTag, LeftBoundTag>::value ||
                      std::is_same<BoundDirectionTag, RightBoundTag>::value,
                  "Incorrect BoundDirectionTag");

    template <typename CorrectTag> void assert_bound_type_tag() const {
        static_assert(std::is_same<BoundTypeTag, CorrectTag>::value,
                      "Incorrect BoundTypeTag");
    }

    double m_lb;
    double m_ub;
    linear_expression m_expr;
};

/// operator<< for Lower Bound
template <typename Stream, typename BoundDirection>
Stream &operator<<(
    Stream &o,
    const single_bounded_expression<LowerBoundTag, BoundDirection> &expr) {
    o << expr.get_lower_bound() << " <= " << expr.get_expression();
    return o;
}

/// operator<< for Upper Bound
template <typename Stream, typename BoundDirection>
Stream &operator<<(
    Stream &o,
    const single_bounded_expression<UpperBoundTag, BoundDirection> &expr) {
    o << expr.get_expression() << " <= " << expr.get_upper_bound();
    return o;
}

/**
 * Double bounded expression class.
 */
class double_bounded_expression {
  public:
    /// Constructor.
    double_bounded_expression()
        : m_lb(lp_traits::MINUS_INF), m_ub(lp_traits::PLUS_INF) {}

    /// Constructor from a single column expression.
    double_bounded_expression(const col_id &expr)
        : m_lb(lp_traits::MINUS_INF), m_ub(lp_traits::PLUS_INF), m_expr(expr) {}

    /// Constructor from expression, lower and upper bound.
    double_bounded_expression(const linear_expression &expr,
                              double lb = lp_traits::MINUS_INF,
                              double ub = lp_traits::PLUS_INF)
        : m_lb(lb), m_ub(ub), m_expr(expr) {}

    /// Constructor from lower bound expression and an upper bound.
    template <typename BoundDirection>
    double_bounded_expression(
        const single_bounded_expression<LowerBoundTag, BoundDirection> &expr,
        double ub = lp_traits::PLUS_INF)
        : m_lb(expr.get_lower_bound()), m_ub(ub),
          m_expr(expr.get_expression()) {}

    /// Constructor from upper bound expression and a lower bound.
    template <typename BoundDirection>
    double_bounded_expression(
        const single_bounded_expression<UpperBoundTag, BoundDirection> &expr,
        double lb = lp_traits::MINUS_INF)
        : m_lb(lb), m_ub(expr.get_upper_bound()),
          m_expr(expr.get_expression()) {}

    /// Expression getter.
    linear_expression get_expression() const { return m_expr; }

    /// Lower bound getter.
    double get_lower_bound() const { return m_lb; }

    /// Upper bound getter.
    double get_upper_bound() const { return m_ub; }

  private:
    double m_lb;
    double m_ub;
    linear_expression m_expr;
};

namespace detail {
template <typename Stream, typename PrintCol>
void print_double_bounded_expression(Stream &o,
                                     const double_bounded_expression &expr,
                                     PrintCol print_col) {
    o << expr.get_lower_bound() << " <= ";
    print_expression(o, expr.get_expression(), print_col);
    o << " <= " << expr.get_upper_bound();
}
} //!detail

/// operator<< for double_bounded_expression
template <typename Stream>
Stream &operator<<(Stream &o, const double_bounded_expression &expr) {
    detail::print_double_bounded_expression(o, expr, detail::col_id_to_string);
    return o;
}

/// double <= linear_expression operator.
inline single_bounded_expression<LowerBoundTag, LeftBoundTag> operator<=(
    double val, const linear_expression &expr) {
    return single_bounded_expression<LowerBoundTag, LeftBoundTag>(val, expr);
}

/// linear_expression >= double operator.
inline single_bounded_expression<LowerBoundTag, RightBoundTag> operator>=(
    const linear_expression &expr, double val) {
    return single_bounded_expression<LowerBoundTag, RightBoundTag>(val, expr);
}

/// double >= linear_expression operator.
inline single_bounded_expression<UpperBoundTag, LeftBoundTag> operator>=(
    double val, const linear_expression &expr) {
    return single_bounded_expression<UpperBoundTag, LeftBoundTag>(expr, val);
}

/// linear_expression <= double operator.
inline single_bounded_expression<UpperBoundTag, RightBoundTag> operator<=(
    const linear_expression &expr, double val) {
    return single_bounded_expression<UpperBoundTag, RightBoundTag>(expr, val);
}

/// double >= right_lower_bound_expression operator.
inline double_bounded_expression operator>=(
    double val,
    const single_bounded_expression<LowerBoundTag, RightBoundTag> &expr) {
    return double_bounded_expression(expr, val);
}

/// left_lower_bound_expression <= double operator.
inline double_bounded_expression operator<=(
    const single_bounded_expression<LowerBoundTag, LeftBoundTag> &expr,
    double val) {
    return double_bounded_expression(expr, val);
}

/// double <= right_upper_bound_expression operator.
inline double_bounded_expression operator<=(
    double val,
    const single_bounded_expression<UpperBoundTag, RightBoundTag> &expr) {
    return double_bounded_expression(expr, val);
}

/// left_upper_bound_expression >= double operator.
inline double_bounded_expression operator>=(
    const single_bounded_expression<UpperBoundTag, LeftBoundTag> &expr,
    double val) {
    return double_bounded_expression(expr, val);
}

/// double == linear_expression operator.
inline double_bounded_expression operator==(double val,
                                            const linear_expression &expr) {
    return double_bounded_expression(expr, val, val);
}

/// linear_expression == double operator.
inline double_bounded_expression operator==(const linear_expression &expr,
                                            double val) {
    return double_bounded_expression(expr, val, val);
}

} // lp
} // paal

#endif // PAAL_CONSTRAINTS_HPP
