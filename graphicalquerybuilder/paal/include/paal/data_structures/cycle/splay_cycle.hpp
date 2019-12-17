//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file splay_cycle.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-03-08
 */
#ifndef PAAL_SPLAY_CYCLE_HPP
#define PAAL_SPLAY_CYCLE_HPP

#include "paal/data_structures/splay_tree.hpp"
#include "paal/data_structures/bimap.hpp"
#include "paal/data_structures/cycle_iterator.hpp"

namespace paal {
namespace data_structures {

/**
 * @brief Cycle based on splay tree
 *
 * @tparam T
 */
template <typename T>
class splay_cycle {
    using SIter = typename splay_tree<T>::iterator;
public:
    typedef cycle_iterator<SIter> VIter;

    splay_cycle() = default;

    /**
     * @brief constructor from range
     *
     * @tparam Iter
     * @param begin
     * @param end
     */
    template <typename Iter>
    splay_cycle(Iter begin, Iter end)
        : m_splay_tree(begin, end), m_size(m_splay_tree.size()) {}

    /**
     * @brief vertices begin
     *
     * @return
     */
    VIter vbegin() const {
        return VIter(m_splay_tree.begin(), m_splay_tree.begin(),
                     m_splay_tree.end());
    }

    /**
     * @brief vertices begin (from t)
     *
     * @param t
     *
     * @return
     */
    VIter vbegin(const T &t) const {
        std::size_t i = m_splay_tree.get_idx(t);
        assert(i != std::size_t(-1));
        return VIter(m_splay_tree.splay(i), m_splay_tree.begin(), m_splay_tree.end());
    }

    /**
     * @brief vertices end
     *
     * @return
     */
    VIter vend() const {
        auto e = m_splay_tree.end();
        return VIter(e, e, e);
    }

    /**
     * @brief flips range from begin to end
     *
     * @param begin
     * @param end
     */
    void flip(const T &begin, const T &end) {
        if (begin == end) {
            return;
        }
        std::size_t b = m_splay_tree.get_idx(begin);
        assert(b != std::size_t(-1));
        std::size_t e = m_splay_tree.get_idx(end);
        assert(e != std::size_t(-1));
        if (b < e) {
            m_splay_tree.reverse(b, e);
        } else {
            m_splay_tree.reverse(e + 1, b - 1);
            m_splay_tree.reverse(0, m_size - 1);
        }
    }

  private:
    splay_tree<T> m_splay_tree;
    const std::size_t m_size;
};

} //! data_structures
} //! paal
#endif // PAAL_SPLAY_CYCLE_HPP
