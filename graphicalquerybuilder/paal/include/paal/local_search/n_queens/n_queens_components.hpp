//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file n_queens_components.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-01-02
 */
#ifndef PAAL_N_QUEENS_COMPONENTS_HPP
#define PAAL_N_QUEENS_COMPONENTS_HPP

#include "paal/data_structures/subset_iterator.hpp"

#include <boost/iterator/function_input_iterator.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace paal {
namespace local_search {

/**
 * @brief class describing Move
 */
struct Move {
    /**
     * @brief constructor
     *
     * @param from
     * @param to
     */
    Move(int from, int to) : m_from(from), m_to(to) {}

    /**
     * @brief getter for m_from
     *
     * @return
     */
    int get_from() const { return m_from; }

    /**
     * @brief getter for m_to
     *
     * @return
     */
    int get_to() const { return m_to; }

  private:
    int m_from;
    int m_to;
};

/**
 * @brief Functor creating Move
 */
struct make_move {
    /**
     * @brief operator()
     *
     * @param from
     * @param to
     *
     * @return
     */
    Move operator()(int from, int to) const { return Move(from, to); }
};

/**
 * @brief n_queens_commit functor
 */
struct n_queens_commit {
    template <typename Solution>
    /**
     * @brief Operator swaps elements of the solution range
     *
     * @param sol
     * @param move
     */
        bool operator()(Solution &sol, Move move) const {
        sol.swap_queens(move.get_from(), move.get_to());
        return true;
    }
};

namespace detail {

struct tuple_to_move {
    using result_type = Move;
    result_type operator()(std::tuple<int, int> t) const {
        return Move(std::get<0>(t), std::get<1>(t));
    }
};
} //!detail

/**
 * @brief n_queensget_moves functor
 */
class n_queensget_moves {

    /**
     * @brief Functor needed for type computation
     *
     * @tparam Solution
     */
    template <typename Solution> struct types_eval {
        using SolutionIter = decltype(std::declval<Solution>().begin());
        using Subset =
            data_structures::subsets_iterator<2, SolutionIter, make_move>;
        using IterPair = std::pair<Subset, Subset>;
        using Range = boost::iterator_range<Subset>;
    };

  public:
    /**
     * @brief operator() returns all the elements
     *
     * @tparam Solution
     * @param solution
     *
     * @return
     */
    template <typename Solution>
    auto operator()(
        const Solution &solution) const->typename types_eval<Solution>::Range {
        return data_structures::make_subsets_iterator_range<2>(
                solution.begin(), solution.end(), make_move{});
    }
};

/**
 * @brief n_queens_gain functor
 */
struct n_queens_gain {
    /**
     * @brief computes difference in cost
     *
     * @tparam Solution
     * @param solution
     * @param move
     *
     * @return
     */
    template <typename Solution>
    int operator()(const Solution &solution, Move move) const {
        int x1 = move.get_from();
        int y1 = solution.get_y(x1);
        int x2 = move.get_to();
        int y2 = solution.get_y(x2);

        return -solution.get_num_attacing(x1, y2) -
               solution.get_num_attacing(x2, y1) +
               solution.get_num_attacing(x1, y1) - 2 +
               solution.get_num_attacing(x2, y2) - 2 -
               2 * (std::abs(x1 - x2) == std::abs(y1 - y2));
    }
};
} //!local_search
} //!paal

#endif // PAAL_N_QUEENS_COMPONENTS_HPP
