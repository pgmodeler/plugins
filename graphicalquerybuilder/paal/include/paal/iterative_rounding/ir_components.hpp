//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2014 Piotr Godlewski
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file ir_components.hpp
 * @brief
 * @author Piotr Wygocki, Piotr Godlewski
 * @version 1.0
 * @date 2013-05-10
 */
#ifndef PAAL_IR_COMPONENTS_HPP
#define PAAL_IR_COMPONENTS_HPP


#include "paal/data_structures/components/components.hpp"
#include "paal/lp/ids.hpp"
#include "paal/lp/lp_base.hpp"
#include "paal/lp/problem_type.hpp"
#include "paal/utils/floating.hpp"
#include "paal/utils/functors.hpp"

#include <boost/optional.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/tuple/tuple.hpp>

#include <cmath>

namespace paal {
namespace ir {

/**
 * @brief Default column rounding condition component.
 */
class default_round_condition {
  public:
    /**
     * @brief constructor takes epsilon used in double comparison.
     */
    default_round_condition(double epsilon = utils::compare<double>::default_epsilon()): m_compare(epsilon) { }

    /**
     * @brief Rounds the column if its value is integral.
     */
    template <typename Problem, typename LP>
    boost::optional<double> operator()(Problem &, const LP &lp,
                                       lp::col_id col) {
        double x = lp.get_col_value(col);
        double r = std::round(x);
        if (m_compare.e(x,r)) {
            return r;
        }
        return boost::none;
    }

  protected:
    /// Double comparison object.
    const utils::compare<double> m_compare;
};

/**
 * @brief Column rounding component.
 *        Rounds a column if its value is equal to one of the template parameter
 * values.
 */
template <int...> class round_condition_equals {
    round_condition_equals() = delete;
};

/**
 * @brief Column rounding component.
 *        Rounds a column if its value is equal to one of the template parameter
 * values.
 */
template <int arg, int... args>
class round_condition_equals<arg,
                             args...> : public round_condition_equals<args...> {
  public:
    /**
     * @brief constructor takes epsilon used in double comparison.
     */
    round_condition_equals(double epsilon = utils::compare<double>::default_epsilon()): round_condition_equals<args...>(epsilon) { }

    /// Rounds a column if its value is equal to one of the template parameter
    /// values.
    template <typename Problem, typename LP>
    boost::optional<double> operator()(Problem &, const LP &lp,
                                       lp::col_id col) {
        return get(lp, lp.get_col_value(col));
    }

  protected:
    /// Checks if the value can be rounded to the first template parameter.
    template <typename LP>
    boost::optional<double> get(const LP & lp, double x) {
        if (this->m_compare.e(x, arg)) {
            return double(arg);
        } else {
            return round_condition_equals<args...>::get(lp, x);
        }
    }
};

/**
 * @brief Column rounding component.
 *        Rounds a column if its value is equal to one of the template parameter
 * values.
 *        Edge case (no template parameter values).
 */
template <> class round_condition_equals<> {
  public:
    /**
     * @brief constructor takes epsilon used in double comparison.
     */
    round_condition_equals(double epsilon = utils::compare<double>::default_epsilon()): m_compare(epsilon) { }

  protected:
    /// Edge case: return false.
    template <typename LP> boost::optional<double> get(const LP &lp, double x) {
        return boost::none;
    }

    /// Double comparison object.
    const utils::compare<double> m_compare;
};

/**
 * @brief Column rounding component.
 *        Rounds a column if its value satisfies a fixed condition.
 *        The column is rounded to a value defined by a fixed function.
 */
template <typename Cond, typename F> class round_condition_to_fun {
  public:
    /**
     * @brief Constructor. Takes the rounding condition and the rounding
     * function.
     */
    round_condition_to_fun(Cond c = Cond(), F f = F()) : m_cond(c), m_f(f) {}

    /// Rounds a column if its value satisfies a fixed condition.
    template <typename Problem, typename LP>
    boost::optional<double> operator()(Problem &, const LP &lp,
                                       lp::col_id col) {
        double x = lp.get_col_value(col);
        if (m_cond(x)) {
            return m_f(x);
        }
        return boost::none;
    }

  private:
    Cond m_cond;
    F m_f;
};

/**
 * @brief Checks if a variable is greater or equal than a fixed bound.
 */
class cond_bigger_equal_than {
  public:
    /**
     * @brief constructor takes epsilon used in double comparison.
     */
    cond_bigger_equal_than(double b, double epsilon = utils::compare<double>::default_epsilon())
        : m_bound(b), m_compare(epsilon) {}

    /// Checks if a variable is greater or equal than a fixed bound.
    bool operator()(double x) { return m_compare.ge(x, m_bound); }

  private:
    double m_bound;
    const utils::compare<double> m_compare;
};

/**
 * @brief Column rounding component.
 *        A variable is rounded up to 1, if it has value at least half in the
 * solution.
 */
struct round_condition_greater_than_half  :
    public round_condition_to_fun<cond_bigger_equal_than, utils::return_one_functor> {
        /**
         * @brief constructor takes epsilon used in double comparison.
         */
        round_condition_greater_than_half(double epsilon = utils::compare<double>::default_epsilon()) :
            round_condition_to_fun(cond_bigger_equal_than(0.5, epsilon)) {}
};

/**
 * @brief Finds an extreme point solution to the LP.
 */
struct default_solve_lp_to_extreme_point {
    /// Finds an extreme point solution to the LP.
    template <typename Problem, typename LP>
    lp::problem_type operator()(Problem &, LP &lp) {
        return lp.solve_simplex(lp::PRIMAL);
    }
};

/**
 * @brief Finds an extreme point solution to the LP.
 */
struct default_resolve_lp_to_extreme_point {
    /// Finds an extreme point solution to the LP.
    template <typename Problem, typename LP>
    lp::problem_type operator()(Problem &, LP &lp) {
        return lp.resolve_simplex(lp::PRIMAL);
    }
};

/**
 * @brief Default stop condition component.
 */
class default_stop_condition {
  public:
    /**
     * @brief Constructor. Takes epsilon used in double comparison.
     */
    default_stop_condition(double epsilon = utils::compare<double>::default_epsilon()): m_compare(epsilon) { }

    /**
     * @brief Checks if the current LP solution has got only integer values.
     */
    template <typename Problem, typename LP>
    bool operator()(Problem &, const LP &lp) {
        for (lp::col_id col : lp.get_columns()) {
            double col_val = lp.get_col_value(col);
            if (!m_compare.e(col_val, std::round(col_val))) {
                return false;
            }
        }

        return true;
    }

  protected:
    /// Double comparison object.
    const utils::compare<double> m_compare;
};

/**
 * @brief Checks if the relaxations limit was reached.
 */
class relaxations_limit_condition {
  public:
    /// Constructor.
    relaxations_limit_condition(int limit = 1) : m_limit(limit) {}

    /**
     * @brief Checks if the relaxations limit was reached.
     */
    bool operator()(int relaxed) { return relaxed >= m_limit; }

  private:
    int m_limit;
};

class Init;
class RoundCondition;
class RelaxCondition;
class SetSolution;
class SolveLP;
class ResolveLP;
class StopCondition;
class RelaxationsLimit;

using components = data_structures::components<
    Init,
    data_structures::NameWithDefault<RoundCondition, default_round_condition>,
    data_structures::NameWithDefault<RelaxCondition, utils::always_false>,
    data_structures::NameWithDefault<SetSolution, utils::skip_functor>,
    data_structures::NameWithDefault<SolveLP, default_solve_lp_to_extreme_point>,
    data_structures::NameWithDefault<ResolveLP, default_resolve_lp_to_extreme_point>,
    data_structures::NameWithDefault<StopCondition, default_stop_condition>,
    data_structures::NameWithDefault<RelaxationsLimit, utils::always_false>>;

/**
 * @brief Iterative rounding components.
 */
template <typename... Args>
using IRcomponents = typename components::type<Args...>;

/**
 * @brief Returns iterative rounding components.
 */
template <typename... Args>
auto make_IRcomponents(Args &&... args)
    ->decltype(components::make_components(std::forward<Args>(args)...)) {
    return components::make_components(std::forward<Args>(args)...);
}

} // ir
} // paal

#endif // PAAL_IR_COMPONENTS_HPP
