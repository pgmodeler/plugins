//=======================================================================
// Copyright (c) 2013 Piotr Wygocki, Piotr Godlewski
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file glp.hpp
 * @brief
 * @author Piotr Godlewski, Piotr Wygocki
 * @version 2.0
 * @date 2014-05-19
 */
#ifndef PAAL_GLP_HPP
#define PAAL_GLP_HPP

#include "paal/data_structures/bimap.hpp"
#include "paal/lp/constraints.hpp"
#include "paal/lp/ids.hpp"
#include "paal/lp/lp_base.hpp"
#include "paal/lp/problem_type.hpp"
#include "paal/utils/irange.hpp"

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <glpk.h>

namespace paal {
namespace lp {

namespace detail {

/**
 * @class glp_impl
 * @brief LP implementation using GLPK.
 */
class glp_impl {
    using Ids = std::vector<int>;
    using Vals = std::vector<double>;
    using IdsVals = std::vector<std::pair<row_id, double>>;
    using RowsInColumnIterator = IdsVals::const_iterator;

  public:
    /**
     * Constructor.
     */
    glp_impl()
        : m_lp(glp_create_prob()), m_total_col_nr(0), m_total_row_nr(0),
          m_idx_cols_tmp(1), m_idx_rows_tmp(1), m_val_cols_tmp(1),
          m_val_rows_tmp(1), m_rows_tmp(1) {
        glp_term_out(GLP_OFF);
        glp_init_smcp(&m_glpk_control);
        glp_load_matrix(m_lp, 0, NULL, NULL, NULL);
        m_glpk_control.msg_lev = GLP_MSG_OFF;
    }

    /**
     * Destructor.
     */
    ~glp_impl() { glp_delete_prob(m_lp); }

    /**
     * Frees GLPK resources, common for all LP instances.
     * Should be called after all LP instances are destructed.
     */
    static void free_env() { glp_free_env(); }

    /**
     * Sets the problem optimization type (min/max).
     */
    void set_optimization_type(optimization_type opt_type) {
        glp_set_obj_dir(m_lp, opt_type == MINIMIZE ? GLP_MIN : GLP_MAX);
    }

    /**
     * Adds a new column to the LP.
     *
     * @param cost_coef coefficient of the column in the objective function
     * @param lb column lower bound value
     * @param ub column upper bound value
     *
     * @return column identifier
     */
    col_id add_column(double cost_coef, double lb, double ub) {
        int colNr = glp_add_cols(m_lp, 1);

        glp_set_col_bnds(m_lp, colNr, bounds_to_glp_type(lb, ub), lb, ub);
        glp_set_obj_coef(m_lp, colNr, cost_coef);
        glp_set_mat_col(m_lp, colNr, 0, NULL, NULL);

        resize_col_tmp();
        ++m_total_col_nr;
        m_col_idx.add(m_total_col_nr - 1);
        return col_id(m_total_col_nr - 1);
    }

    /**
     * Adds a new row to the LP.
     *
     * @param constraint constraint being added
     *
     * @return row identifier
     */
    row_id add_row(const double_bounded_expression &constraint) {
        int rowNr = glp_add_rows(m_lp, 1);
        auto lb = constraint.get_lower_bound();
        auto ub = constraint.get_upper_bound();
        glp_set_row_bnds(m_lp, rowNr, bounds_to_glp_type(lb, ub), lb, ub);

        resize_row_tmp();
        ++m_total_row_nr;
        m_row_idx.add(m_total_row_nr - 1);
        auto row = row_id(m_total_row_nr - 1);

        set_row_expression(row, constraint.get_expression());

        return row;
    }

    /**
     * Sets the lower bound of an existing LP column.
     *
     * @param col column identifier
     * @param lb row lower bound value
     */
    void set_col_lower_bound(col_id col, double lb) {
        auto ub = get_col_upper_bound(col);
        glp_set_col_bnds(m_lp, get_col(col), bounds_to_glp_type(lb, ub), lb,
                         ub);
    }

    /**
     * Sets the upper bound of an existing LP column.
     *
     * @param col column identifier
     * @param ub row upper bound value
     */
    void set_col_upper_bound(col_id col, double ub) {
        auto lb = get_col_lower_bound(col);
        glp_set_col_bnds(m_lp, get_col(col), bounds_to_glp_type(lb, ub), lb,
                         ub);
    }

    /**
     * Sets the cost coefficient of an existing LP column.
     *
     * @param col column identifier
     * @param cost_coef new cost coefficient
     */
    void set_col_cost(col_id col, double cost_coef) {
        glp_set_obj_coef(m_lp, get_col(col), cost_coef);
    }

    /**
     * Sets the lower bound of an existing LP row.
     *
     * @param row row identifier
     * @param lb row lower bound value
     */
    void set_row_lower_bound(row_id row, double lb) {
        auto ub = get_row_upper_bound(row);
        glp_set_row_bnds(m_lp, get_row(row), bounds_to_glp_type(lb, ub), lb,
                         ub);
    }

    /**
     * Sets the upper bound of an existing LP row.
     *
     * @param row row identifier
     * @param ub row upper bound value
     */
    void set_row_upper_bound(row_id row, double ub) {
        auto lb = get_row_lower_bound(row);
        glp_set_row_bnds(m_lp, get_row(row), bounds_to_glp_type(lb, ub), lb,
                         ub);
    }

    /**
     * Sets the expression of an existing row.
     *
     * @param row row identifier
     * @param expr new expression
     */
    void set_row_expression(row_id row, const linear_expression &expr) {
        auto elems = expr.get_elements();
        int expr_size = boost::distance(elems);

        for (auto elem : elems | boost::adaptors::indexed(1)) {
            m_idx_cols_tmp[elem.index()] = get_col(elem.value().first);
            m_val_cols_tmp[elem.index()] = elem.value().second;
        }

        glp_set_mat_row(m_lp, get_row(row), expr_size, &m_idx_cols_tmp[0],
                        &m_val_cols_tmp[0]);
    }

    /**
     * Removes a column form the LP.
     *
     * @param col ID of the column to be removed
     */
    void delete_col(col_id col) {
        int arr[2];
        arr[1] = get_col(col);
        m_col_idx.erase(col.get());
        glp_del_cols(m_lp, 1, arr);
    }

    /**
     * Removes a row form the LP.
     *
     * @param row ID of the row to be removed
     */
    void delete_row(row_id row) {
        int arr[2];
        arr[1] = get_row(row);
        m_row_idx.erase(row.get());
        glp_del_rows(m_lp, 1, arr);
    }

    /**
     * Solves the LP using the primal simplex method.
     *
     * @param type simplex type (primal / dual)
     *
     * @return solution status
     */
    problem_type solve_simplex(simplex_type type = PRIMAL) {
        return run_simplex(type, false);
    }

    /**
     * Resolves the LP (starting from the previously found solution)
     * using the primal simplex method.
     *
     * @param type simplex type (primal / dual)
     *
     * @return solution status
     */
    problem_type resolve_simplex(simplex_type type = PRIMAL) {
        return run_simplex(type, true);
    }

    /**
     * Returns the found objective function value.
     * Should be called only after the LP has been solved and if it
     * wasn't modified afterwards.
     */
    double get_obj_value() const { return glp_get_obj_val(m_lp); }

    /**
     * Returns column primal value.
     * Should be called only after the LP has been solved and if it
     * wasn't modified afterwards.
     */
    double get_col_value(col_id col) const {
        return glp_get_col_prim(m_lp, get_col(col));
    }

    /**
     * Returns the column cost function coefficient.
     */
    double get_col_coef(col_id col) const {
        return glp_get_obj_coef(m_lp, get_col(col));
    }

    /**
     * Returns the column lower bound.
     */
    double get_col_lower_bound(col_id col) const {
        return get_col_bounds(col).first;
    }

    /**
     * Returns the column upper bound.
     */
    double get_col_upper_bound(col_id col) const {
        return get_col_bounds(col).second;
    }

    /**
     * Returns row dual value.
     * Should be called only after the LP has been solved and if it
     * wasn't modified afterwards.
     */
    double get_row_dual_value(row_id row) const {
       return glp_get_row_dual(m_lp, get_row(row));
    }

    /**
     * Returns the row lower bound.
     */
    double get_row_lower_bound(row_id row) const {
        return get_row_bounds(row).first;
    }

    /**
     * Returns the row upper bound.
     */
    double get_row_upper_bound(row_id row) const {
        return get_row_bounds(row).second;
    }

    /**
     * Returns the expression of an existing row.
     */
    linear_expression get_row_expression(row_id row) const {
        linear_expression exp;
        int size = glp_get_mat_row(m_lp, get_row(row), &m_idx_cols_tmp[0],
                                   &m_val_cols_tmp[0]);
        for (auto i : irange(1, size + 1)) {
            exp += m_val_cols_tmp[i] * get_col_id(m_idx_cols_tmp[i]);
        }
        return exp;
    }

    /**
     * Returns the identifiers and coefficients of all rows in a given column,
     * which constraint matrix coefficient is non-zero (as an iterator range).
     */
    boost::iterator_range<RowsInColumnIterator>
    get_rows_in_column(col_id col) const {
        int size = glp_get_mat_col(m_lp, get_col(col), &m_idx_rows_tmp[0],
                                   &m_val_rows_tmp[0]);
        for (auto i : irange(1, size + 1)) {
            m_rows_tmp[i].first = get_row_id(m_idx_rows_tmp[i]);
            m_rows_tmp[i].second = m_val_rows_tmp[i];
        }
        return boost::make_iterator_range(m_rows_tmp.begin() + 1,
                                          m_rows_tmp.begin() + size + 1);
    }

  private:
    glp_impl(glp_impl &&) {}
    glp_impl(const glp_impl &) {}

    /**
     * Determines the GLPK bound type based on the lower and upper bound values.
     */
    static int bounds_to_glp_type(double lb, double ub) {
        if (lb == ub) {
            return GLP_FX;
        } else if (lb == lp_traits::MINUS_INF) {
            if (ub == lp_traits::PLUS_INF) {
                return GLP_FR;
            } else {
                return GLP_UP;
            }
        } else {
            if (ub == lp_traits::PLUS_INF) {
                return GLP_LO;
            } else {
                return GLP_DB;
            }
        }
    }

    /**
     * Determines the lower and upper bound values based on the GLPK bound type
     * and GLPK returned values.
     */
    static std::pair<double, double> glp_type_to_bounds(double lb, double ub,
                                                        int type) {
        switch (type) {
        case GLP_FX:
            return { lb, lb };
        case GLP_FR:
            return { lp_traits::MINUS_INF, lp_traits::PLUS_INF };
        case GLP_UP:
            return { lp_traits::MINUS_INF, ub };
        case GLP_LO:
            return { lb, lp_traits::PLUS_INF };
        case GLP_DB:
            return { lb, ub };
        default:
            assert(false);
            return { 0, 0 };
        }
    }

    /**
     * Determines the GLPK simplex type based on the paal::lp simplex type.
     */
    static int simplex_type_to_glp(simplex_type type) {
        switch (type) {
        case PRIMAL:
            return GLP_PRIMAL;
        case DUAL:
            return GLP_DUAL;
        default:
            assert(false);
            return GLP_PRIMAL;
        }
    }

    /**
     * Returns column bounds.
     */
    std::pair<double, double> get_col_bounds(col_id col) const {
        auto column = get_col(col);
        return glp_type_to_bounds(glp_get_col_lb(m_lp, column),
                                  glp_get_col_ub(m_lp, column),
                                  glp_get_col_type(m_lp, column));
    }

    /**
     * Returns row bounds.
     */
    std::pair<double, double> get_row_bounds(row_id row) const {
        auto row_num = get_row(row);
        return glp_type_to_bounds(glp_get_row_lb(m_lp, row_num),
                                  glp_get_row_ub(m_lp, row_num),
                                  glp_get_row_type(m_lp, row_num));
    }

    /**
     * Converts column identifier to the GLPK column index.
     */
    int get_col(col_id col) const { return m_col_idx.get_idx(col.get()) + 1; }

    /**
     * Converts row identifier to the GLPK row index.
     */
    int get_row(row_id row) const { return m_row_idx.get_idx(row.get()) + 1; }

    /**
     * Converts GLPK column index to the column identifier.
     */
    col_id get_col_id(int col) const {
        return col_id(m_col_idx.get_val(col - 1));
    }

    /**
     * Converts GLPK row index to the row identifier.
     */
    row_id get_row_id(int row) const {
        return row_id(m_row_idx.get_val(row - 1));
    }

    /**
     * Optimizes the LP using the simplex method.
     *
     * @return solution status
     */
    problem_type run_simplex(simplex_type type, bool resolve) {
        m_glpk_control.meth = simplex_type_to_glp(type);
        static const std::string buggy_version = "4.52";
        if (!resolve || glp_version() == buggy_version) {
            //TODO waiting for response to on
            //http://lists.gnu.org/archive/html/bug-glpk/2014-06/msg00000.html
            glp_adv_basis(m_lp, 0);
        }
        int ret = glp_simplex(m_lp, &m_glpk_control);
        if (resolve && ret != 0) {
            // if basis is not valid, create basis and try again
            glp_adv_basis(m_lp, 0);
            ret = glp_simplex(m_lp, &m_glpk_control);
        }
        assert(ret == 0);
        return get_primal_type();
    }

    /**
     * Converts the GLPK soltion status into paal::lp::problem_type.
     *
     * @return solution status
     */
    problem_type get_primal_type() {
        if (glp_get_status(m_lp) == GLP_OPT) {
            return OPTIMAL;
        }

        switch (glp_get_prim_stat(m_lp)) {
        case GLP_UNDEF:
            return UNDEFINED;
        case GLP_NOFEAS:
            return INFEASIBLE;
        case GLP_FEAS:
        case GLP_INFEAS:
            if (glp_get_dual_stat(m_lp) == GLP_NOFEAS) {
                return UNBOUNDED;
            } else {
                return UNDEFINED;
            }
        default:
            assert(false);
            return UNDEFINED;
        }
    }

    void resize_col_tmp() {
        m_idx_cols_tmp.push_back(0);
        m_val_cols_tmp.push_back(0);
    }

    void resize_row_tmp() {
        m_idx_rows_tmp.push_back(0);
        m_val_rows_tmp.push_back(0);
        m_rows_tmp.push_back({ row_id(0), 0 });
    }

    /// GLPK problem object
    glp_prob *m_lp;
    glp_smcp m_glpk_control;

    /// mapping between GLPK column numbers and column IDs
    data_structures::eraseable_bimap<int> m_col_idx;
    /// mapping between GLPK row numbers and column IDs
    data_structures::eraseable_bimap<int> m_row_idx;

    int m_total_col_nr;
    int m_total_row_nr;

    mutable Ids m_idx_cols_tmp;
    mutable Ids m_idx_rows_tmp;
    mutable Vals m_val_cols_tmp;
    mutable Vals m_val_rows_tmp;
    mutable IdsVals m_rows_tmp;
};
} // detail

using glp = detail::lp_base<detail::glp_impl>;

} // lp
} // paal

#endif // PAAL_GLP_HPP
