//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file tabu_list.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-01-09
 */
#ifndef PAAL_TABU_LIST_HPP
#define PAAL_TABU_LIST_HPP

#include <boost/functional/hash.hpp>

#include <unordered_set>
#include <queue>

namespace paal {
namespace data_structures {

/**
 * @brief This Tabu list remember some number of last moves
 *
 * @tparam Move
 */
template <typename Move> struct tabu_list_remember_move {

    /**
     * @brief tabu_list_remember_move constructor
     *
     * @param size
     */
    tabu_list_remember_move(unsigned size)
        : m_size(size), m_forbiden_moves_set(size) {}

    /**
     * @brief is tabu member function
     *
     * @tparam Solution
     * @param move
     *
     * @return
     */
    template <typename Solution>
    bool is_tabu(const Solution &, Move move) const {
        return is_tabu(std::move(move));
    }

    /**
     * @brief accept member function
     *
     * @tparam Solution
     * @param move
     */
    template <typename Solution> void accept(const Solution &, Move move) {
        assert(!is_tabu(move));
        m_forbiden_moves_set.insert(move);
        if (m_forbiden_moves_fifo.size() == m_size) {
            m_forbiden_moves_set.erase(m_forbiden_moves_fifo.front());
            m_forbiden_moves_fifo.pop_front();
        }
        m_forbiden_moves_fifo.push_back(std::move(move));
    }

  private:
    /**
     * @brief is tabu does not depend on Solution here
     *
     * @param move
     *
     * @return
     */
    bool is_tabu(const Move &move) const {
        return m_forbiden_moves_set.find(move) != m_forbiden_moves_set.end();
    }

    unsigned m_size;
    std::unordered_set<Move, boost::hash<Move>> m_forbiden_moves_set;
    std::deque<Move> m_forbiden_moves_fifo;
};

/**
 * @brief This Tabu list remember both current solution and move
 *        It is implemented as tabu_list_remember_move<pair<Solution, Move>>
 * with nullptr passed as dummy solution
 *
 * @tparam Solution
 * @tparam Move
 */
template <typename Solution, typename Move>
class tabu_list_remember_solution_and_move
    : tabu_list_remember_move<std::pair<Solution, Move>> {
    typedef tabu_list_remember_move<std::pair<Solution, Move>> base;

  public:
    /**
     * @brief constructor
     *
     * @param size
     */
    tabu_list_remember_solution_and_move(unsigned size) : base(size) {}

    /**
     * @brief is_tabu redirects work to base class
     *
     * @param s
     * @param move
     *
     * @return
     */
    bool is_tabu(Solution s, Move move) const {
        return base::is_tabu(nullptr,
                             std::make_pair(std::move(s), std::move(move)));
    }

    /**
     * @brief accept redirects work to base class
     *
     * @param s
     * @param move
     */
    void accept(Solution &s, const Move &move) {
        base::accept(nullptr, std::make_pair(std::move(s), std::move(move)));
    }
};

} //!data_structures
} //!paal

#endif // PAAL_TABU_LIST_HPP
