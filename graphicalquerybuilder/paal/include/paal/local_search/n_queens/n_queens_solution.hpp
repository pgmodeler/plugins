//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file n_queens_solution.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-01-02
 */
#ifndef PAAL_N_QUEENS_SOLUTION_HPP
#define PAAL_N_QUEENS_SOLUTION_HPP

#include "paal/utils/irange.hpp"

#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/numeric.hpp>

#include <vector>

namespace paal {
namespace local_search {

/**
 * @brief Adapts vector representing n queen problem to class
 * able to efficiently compute gain of given move
 *
 * @tparam NQueensPositionsVector
 */
template <typename NQueensPositionsVector> struct n_queens_solution_adapter {
    typedef typename boost::counting_iterator<int> QueensIterator;

    /**
     * @brief constructor
     *
     * @param pos
     */
    n_queens_solution_adapter(NQueensPositionsVector &pos)
        : m_queen_position(pos),
          m_numeber_attacing_diagonal_negative(boost::distance(pos), 0),
          m_numeber_attacing_diagonal_nonnegative(boost::distance(pos), 0),
          m_numeber_attacing_counter_diagonal(2 * boost::distance(pos), 0) {
        for (auto q : irange(boost::distance(pos))) {
            increase(q);
        }
    }

    /**
     * @brief begin of the queens positions' collection
     *
     * @return
     */
    QueensIterator begin() const { return QueensIterator(0); }

    /**
     * @brief end of the queens positions' collection
     *
     * @return
     */
    QueensIterator end() const {
        return QueensIterator(m_queen_position.size());
    }

    /**
     * @brief swaps two queens positions
     *
     * @param xLeft
     * @param xRight
     */
    void swap_queens(int xLeft, int xRight) {
        int leftPosition = m_queen_position[xLeft];
        put_queen(xLeft, m_queen_position[xRight]);
        put_queen(xRight, leftPosition);
    }

    /**
     * @brief get number of queens attacing (x,y) position
     *
     * @param x
     * @param y
     *
     * @return
     */
    int get_num_attacing(int x, int y) const {
        return m_numeber_attacing_counter_diagonal[x + y] + get_diagonal(x, y);
    }

    /**
     * @brief return y for xth queen
     *
     * @param x
     *
     * @return
     */
    int get_y(int x) const { return m_queen_position[x]; }

    /**
     * @brief computes total number of conflicts on the board
     *
     * @return
     */
    int obj_fun() const {
        auto attacingNr = [](int sum, int n) { return sum + n * (n - 1) / 2; };
        int sum = boost::accumulate(m_numeber_attacing_counter_diagonal, 0,
                                    attacingNr);
        sum = boost::accumulate(m_numeber_attacing_diagonal_negative, sum,
                                attacingNr);
        return boost::accumulate(m_numeber_attacing_diagonal_nonnegative, sum,
                                 attacingNr);
    }

  private:

    /**
     * @brief puts xth queen on position y
     *
     * @param x
     * @param y
     */
    void put_queen(int x, int y) {
        decrease(x);
        m_queen_position[x] = y;
        increase(x);
    }

    /**
     * @brief get diagonal counter for diagonal of the xth queen
     *
     * @param x
     *
     * @return
     */
    int &get_diagonal(int x) { return get_diagonal(x, m_queen_position[x]); }

    /**
     * @brief gets diagonal crossing (x,y) position
     *
     * @param x
     * @param y
     *
     * @return
     */
    int &get_diagonal(int x, int y) {
        if (x >= y) {
            return m_numeber_attacing_diagonal_negative[x - y];
        } else {
            return m_numeber_attacing_diagonal_nonnegative[y - x];
        }
    }

    /**
     * @brief const version of get_diagonal(x,y)
     *
     * @param x
     * @param y
     *
     * @return
     */
    int get_diagonal(int x, int y) const {
        if (x >= y) {
            return m_numeber_attacing_diagonal_negative[x - y];
        } else {
            return m_numeber_attacing_diagonal_nonnegative[y - x];
        }
    }

    /**
     * @brief decrease diagonals counters fo xth queen
     *
     * @param x
     */
    void decrease(int x) {
        --m_numeber_attacing_counter_diagonal[x + m_queen_position[x]];
        --get_diagonal(x);
    }

    /**
     * @brief decrese diagonal counter for the xth queen
     *
     * @param x
     */
    void increase(int x) {
        ++m_numeber_attacing_counter_diagonal[x + m_queen_position[x]];
        ++get_diagonal(x);
    }

    NQueensPositionsVector &m_queen_position;
    std::vector<int> m_numeber_attacing_diagonal_negative;
    std::vector<int> m_numeber_attacing_diagonal_nonnegative;
    std::vector<int> m_numeber_attacing_counter_diagonal;
};

} //!local_search
} //!paal

#endif // PAAL_N_QUEENS_SOLUTION_HPP
