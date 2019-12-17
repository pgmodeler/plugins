//=======================================================================
// Copyright (c) 2013 Piotr Wygocki, Piotr Godlewski
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file lp_base.hpp
 * @brief
 * @author Piotr Godlewski, Piotr Wygocki
 * @version 1.0
 * @date 2014-04-02
 */
#ifndef PAAL_LP_BASE_HPP
#define PAAL_LP_BASE_HPP

#include "paal/lp/constraints.hpp"
#include "paal/lp/ids.hpp"
#include "paal/lp/problem_type.hpp"

#include <boost/range/iterator_range.hpp>

#include <cassert>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace paal {
namespace lp {

/// optimization type
enum optimization_type {
    MINIMIZE,
    MAXIMIZE
};

/// simplex method type
enum simplex_type {
    PRIMAL,
    DUAL
};

namespace detail {

/**
 * @class lp_base
 * @brief The common LP solvers base class.
 *  Responsible for:
 *   - storing row and column IDs and names
 *   - returning row/col iterator ranges
 *   - calculating some LP parameters, such as: row/col degree, row sum
 * @tparam LP
 */
template <typename LP> class lp_base : public LP {
    using RowSet = std::unordered_set<row_id>;
    using ColSet = std::unordered_set<col_id>;
    using RowNames = std::unordered_map<row_id, std::string>;
    using ColNames = std::unordered_map<col_id, std::string>;

  public:

    using RowIter = RowSet::const_iterator;
    using ColIter = ColSet::const_iterator;

    /**
     * Constructor.
     */
    lp_base(const std::string problem_name = "",
            optimization_type opt_type = MINIMIZE)
        : LP(), m_problem_name(problem_name) {
        LP::set_optimization_type(opt_type);
    }

    /**
     * Sets the problem name.
     */
    void set_lp_name(const std::string problem_name) {
        m_problem_name = problem_name;
    }

    /**
     * Adds a new column to the LP.
     *
     * @param cost_coef coefficient of the column in the objective function
     * @param lb column lower bound value
     * @param ub column upper bound value
     * @param name column symbolic name
     *
     * @return column identifier
     */
    col_id add_column(double cost_coef = 0, double lb = 0.,
                      double ub = lp_traits::PLUS_INF,
                      const std::string &name = "") {
        col_id colId = LP::add_column(cost_coef, lb, ub);
        m_col_ids.insert(colId);
        m_col_names.insert(std::make_pair(colId, name));
        return colId;
    }

    /**
     * Adds a new row to the LP.
     *
     * @param constraint constraint being added
     * @param name row symbolic name
     *
     * @return row identifier
     */
    row_id add_row(const double_bounded_expression &constraint =
                       double_bounded_expression{},
                   const std::string &name = "") {
        row_id rowId = LP::add_row(constraint);
        m_row_ids.insert(rowId);
        m_row_names.insert(std::make_pair(rowId, name));
        return rowId;
    }

    /**
     * Returns the number of columns in the instance.
     */
    int columns_number() const { return m_col_ids.size(); }

    /**
     * Returns the number of rows in the instance.
     */
    int rows_number() const { return m_row_ids.size(); }

    /**
     * Returns the column symbolic name.
     */
    std::string get_col_name(col_id col) const {
        auto it = m_col_names.find(col);
        assert(it != m_col_names.end());
        return it->second;
    }

    /**
     * Returns the row symbolic name.
     */
    std::string get_row_name(row_id row) const {
        auto it = m_row_names.find(row);
        assert(it != m_row_names.end());
        return it->second;
    }

    /**
     * Removes a column from the LP.
     *
     * @param col iterator of the column to be removed
     *
     * @return iterator following the removed column
     */
    ColIter delete_col(ColIter col) {
        LP::delete_col(*col);
        m_col_names.erase(*col);
        return m_col_ids.erase(col);
    }

    /**
     * Removes a column from the LP.
     *
     * @param col ID of the column to be removed
     */
    void delete_col(col_id col) {
        LP::delete_col(col);
        m_col_names.erase(col);
        std::size_t nr = m_col_ids.erase(col);
        assert(nr == 1);
    }

    /**
     * Removes a row form the LP.
     *
     * @param row iterator of the row to be removed
     *
     * @return iterator following the removed row
     */
    RowIter delete_row(RowIter row) {
        LP::delete_row(*row);
        m_row_names.erase(*row);
        return m_row_ids.erase(row);
    }

    /**
     * Removes a row from the LP.
     *
     * @param row ID of the column to be removed
     */
    void delete_row(row_id row) {
        LP::delete_row(row);
        m_row_names.erase(row);
        std::size_t nr = m_row_ids.erase(row);
        assert(nr == 1);
    }

    /**
     * Clears the LP instance.
     */
    void clear() {
        for (auto &&row : m_row_ids) {
            LP::delete_row(row);
        }
        m_row_names.clear();
        m_row_ids.clear();

        for (auto &&col : m_col_ids) {
            LP::delete_col(col);
        }
        m_col_names.clear();
        m_col_ids.clear();
    }

    /**
     * Returns all columns (as an iterator range).
     */
    const ColSet &get_columns() const { return m_col_ids; }

    /**
     * Returns all rows (as an iterator range).
     */
    const RowSet &get_rows() const { return m_row_ids; }

    /**
     * Returns column primal value.
     * Should be called only after the LP has been solved and if it
     * wasn't modified afterwards.
     */
    double get_col_value(col_id col) const {
        return std::min(
                std::max(LP::get_col_value(col),
                    LP::get_col_lower_bound(col)),
                LP::get_col_upper_bound(col));
    }

    /**
     * Returns the number of non-zero coefficients in the given LP matrix
     * column.
     */
    int get_col_degree(col_id col) const {
        return boost::distance(LP::get_rows_in_column(col));
    }

    /**
     * Returns the number of non-zero coefficients in the given LP matrix row.
     */
    int get_row_degree(row_id row) const {
        return LP::get_row_expression(row).non_zeros();
    }

    /**
     * Returns the sum of the values of those columns multiplied by the
     * coefficients
     * in the given LP row.
     */
    double get_row_sum(row_id row) const {
        auto expr = LP::get_row_expression(row);
        auto ids = expr.get_elements();
        return get_row_sum_for_ids(ids.begin(), ids.end());
    }

    /**
     * Output stream operator for printing debug information.
     */
    template <typename ostream>
    friend ostream &operator<<(ostream &o, const lp_base<LP> &lp) {
        o << "Problem name: " << lp.m_problem_name << std::endl;
        auto get_name = [&](col_id col) {
            auto name = " " + lp.get_col_name(col);
            if (name == " ") {
                name = detail::col_id_to_string(col);
            }
            return name;
        };

        o << std::endl << "Objective function:" << std::endl;
        print_collection(
            o, lp.get_columns() | boost::adaptors::transformed([&](col_id col) {
            return pretty_to_string(lp.get_col_coef(col)) + get_name(col);
        }),
            " + ");

        o << std::endl << std::endl << "Columns:" << std::endl;
        for (auto col : lp.get_columns()) {
            o << lp.get_col_lower_bound(col) << " <= " << get_name(col)
              << " <= " << lp.get_col_upper_bound(col) << std::endl;
        }
        o << std::endl << "Rows:" << std::endl;

        for (auto row : lp.get_rows()) {
            auto cols = lp.get_row_expression(row);
            if (cols.non_zeros() == 0) {
                continue;
            }
            o << "Row " << lp.get_row_name(row) << std::endl;
            print_double_bounded_expression(
                o, double_bounded_expression(std::move(cols),
                                             lp.get_row_lower_bound(row),
                                             lp.get_row_upper_bound(row)),
                get_name);
            o << std::endl << std::endl;
        }
        o << std::endl << "Current solution: " << std::endl;
        print_collection(
            o, lp.get_columns() | boost::adaptors::transformed([&](col_id col) {
            return get_name(col) + " = " +
                   pretty_to_string(lp.get_col_value(col));
        }),
            ", ");
        o << std::endl;

        return o;
    }

  private:
    lp_base(lp_base &&) {}
    lp_base(const lp_base &) {}

    template <typename Iter>
    double get_row_sum_for_ids(Iter begin, Iter end) const {
        return std::accumulate(
            begin, end, 0., [ = ](double sum, std::pair<col_id, double> col) {
            return sum + LP::get_col_value(col.first) * col.second;
        });
    }


    std::string m_problem_name;

    ColSet m_col_ids;
    RowSet m_row_ids;

    ColNames m_col_names;
    RowNames m_row_names;
};

} // detail
} // lp
} // paal

#endif // PAAL_LP_BASE_HPP
