//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file simple_cycle.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#ifndef PAAL_SIMPLE_CYCLE_HPP
#define PAAL_SIMPLE_CYCLE_HPP

#include "paal/data_structures/bimap.hpp"

#include <cassert>
#include <vector>
#include <iterator>

namespace paal {
namespace data_structures {

/**
 * @class simple_cycle
 * @brief This is the simplest implementation of the \ref cycle concept based on
 * the list.
 *
 * @tparam CycleEl
 * @tparam IdxT
 */
template <typename CycleEl, typename IdxT = int> class simple_cycle {
  public:
    using cycle_el_pair = std::pair<CycleEl, CycleEl>;
    using cycle_element = CycleEl;

    /**
     * @brief constructor
     *
     * @tparam Iter
     * @param begin
     * @param end
     */
    template <typename Iter> simple_cycle(Iter begin, Iter end) {
        if (begin == end) {
            return;
        }

        std::size_t size = std::distance(begin, end);

        m_predecessor_map.reserve(size);
        m_successor_map.reserve(size);

        IdxT prev_idx = add(*(begin++));
        IdxT firstIdx = prev_idx;
        for (; begin != end; ++begin) {
            IdxT lastIdx = add(*begin);
            link(prev_idx, lastIdx);
            prev_idx = lastIdx;
        }
        link(prev_idx, firstIdx);
    }

    /// after flip the order will be reversed, ie it will be from 'end'  to
    /// 'begin'
    void flip(const CycleEl &begin, const CycleEl &end) {
        IdxT e1 = to_idx(begin);
        IdxT b1 = prev_idx(e1);
        IdxT b2 = to_idx(end);
        IdxT e2 = next_idx(b2);

        partial_reverse(b2, e1);
        link(b1, b2);
        link(e1, e2);
    }

    /**
     * @brief number of elements in the cycle
     *
     * @return
     */
    std::size_t size() const { return m_predecessor_map.size(); }

    /**
     * @brief next element in the cycle
     *
     * @param ce
     *
     * @return
     */
    CycleEl next(const CycleEl &ce) const {
        return from_idx(next_idx(to_idx(ce)));
    }

    // TODO use iterator_fascade
    /**
     * @brief iterator over vertices of the cycle
     */
    class vertex_iterator
        : public std::iterator<std::forward_iterator_tag, CycleEl, ptrdiff_t,
                               CycleEl *, const CycleEl &> {
      public:

        /**
         * @brief constructor
         *
         * @param cm
         * @param ce
         */
        vertex_iterator(const simple_cycle &cm, CycleEl ce)
            : m_cycle(&cm), m_idx(m_cycle->to_idx(ce)), m_first(m_idx) {}

        /**
         * @brief default constructor
         */
        vertex_iterator() : m_cycle(NULL), m_idx(-1) {}

        /**
         * @brief operator++()
         *
         * @return
         */
        vertex_iterator &operator++() {
            m_idx = next_idx(m_idx);

            if (m_idx == m_first) {
                m_idx = -1;
            }

            return *this;
        }

        /**
         * @brief operator++(int)
         *
         * @return
         */
        vertex_iterator operator++(int) {
            vertex_iterator i(*this);
            operator++();
            return i;
        }

        /**
         * @brief operator!=
         *
         * @param ei
         *
         * @return
         */
        bool operator!=(vertex_iterator ei) const { return !operator==(ei); }

        /**
         * @brief operator==
         *
         * @param ei
         *
         * @return
         */
        bool operator==(vertex_iterator ei) const { return m_idx == ei.m_idx; }

        /**
         * @brief operator->()
         *
         * @return
         */
        const CycleEl *const operator->() const { return &operator*(); }

        /**
         * @brief operator*()
         *
         * @return
         */
        const CycleEl &operator*() const { return m_cycle->from_idx(m_idx); }

      private:

        /**
         * @brief next element in the cycle
         *
         * @param i index of the element
         *
         * @return
         */
        IdxT next_idx(IdxT i) const { return m_cycle->next_idx(i); }

        const simple_cycle *m_cycle;
        IdxT m_idx;
        IdxT m_first;
    };

    using vertices =  boost::iterator_range<vertex_iterator>;

    /**
     * @brief begin of the vertices range starting at el
     *
     * @param el
     *
     * @return
     */
    vertex_iterator vbegin(const CycleEl &el) const {
        return vertex_iterator(*this, el);
    }

    /**
     * @brief begin of the vertices range
     *
     * @return
     */
    vertex_iterator vbegin() const { return vbegin(from_idx(0)); }

    /**
     * @brief end of the vertices range
     *
     * @return
     */
    vertex_iterator vend() const { return vertex_iterator(); }

    /**
     * @brief returns range of vertices starting at el
     *
     * @param el
     *
     * @return
     */
    vertices get_vertices_range(const CycleEl &el) const {
        return vertices(vbegin(el), vend());
    }

    /**
     * @brief returns range of vertices
     *
     * @return
     */
    vertices get_vertices_range() const {
        return get_vertices_range(from_idx(0));
    }

    // TODO use iterator_fascade
    /**
     * @brief Iterator on cycle edges
     */
    class edge_iterator
        : public std::iterator<std::forward_iterator_tag, cycle_el_pair,
                               ptrdiff_t, cycle_el_pair *, const cycle_el_pair &> {
      public:
        /**
         * @brief constructor
         *
         * @param cm
         * @param ce
         */
        edge_iterator(const simple_cycle &cm, CycleEl ce)
            : m_cycle(&cm), m_idx(m_cycle->to_idx(ce)), m_first(m_idx) {

            move_curr();
        }

        /**
         * @brief default constructor
         */
        edge_iterator() : m_cycle(NULL), m_idx(-1) {}

        /**
         * @brief operator++()
         *
         * @return
         */
        edge_iterator &operator++() {
            m_idx = next_idx(m_idx);
            move_curr();

            if (m_idx == m_first) {
                m_idx = -1;
            }

            return *this;
        }

        /**
         * @brief operator++(int)
         *
         * @return
         */
        edge_iterator operator++(int) {
            edge_iterator i(*this);
            operator++();
            return i;
        }

        /**
         * @brief operator!=
         *
         * @param ei
         *
         * @return
         */
        bool operator!=(edge_iterator ei) const { return !operator==(ei); }

        /**
         * @brief operator==
         *
         * @param ei
         *
         * @return
         */
        bool operator==(edge_iterator ei) const { return m_idx == ei.m_idx; }

        /**
         * @brief operator->
         *
         * @return
         */
        const cycle_el_pair *const operator->() const { return &m_curr; }

        /**
         * @brief operator*()
         *
         * @return
         */
        const cycle_el_pair &operator*() const { return m_curr; }

      private:
        /**
         * @brief move to the next edge
         */
        void move_curr() {
            m_curr.first = m_cycle->from_idx(m_idx);
            m_curr.second = m_cycle->from_idx(next_idx(m_idx));
        }

        /**
         * @brief gets next id in the cycle
         *
         * @param i
         *
         * @return
         */
        IdxT next_idx(IdxT i) const { return m_cycle->next_idx(i); }

        const simple_cycle *m_cycle;
        IdxT m_idx;
        IdxT m_first;
        cycle_el_pair m_curr;
    };

    using edges = boost::iterator_range<edge_iterator>;

    /**
     * @brief returns edges range starting at el
     *
     * @param el
     *
     * @return
     */
    edges get_edge_range(const CycleEl &el) const {
        return edges(edge_iterator(*this, el), edge_iterator());
    }

    /**
     * @brief returns edges range
     *
     * @return
     */
    edges get_edge_range() const {
        return get_edge_range(from_idx(0));
    }

  protected:
    /**
     * @brief connects two vertices represented by ids
     *
     * @param x
     * @param y
     */
    void link(IdxT x, IdxT y) {
        m_successor_map[x] = y;
        m_predecessor_map[y] = x;
    }

    /**
     * @brief after this operation links from x to y are connected i reverse
     * order, after this function call cycle is in inconsistent state
     *
     * @param x
     * @param y
     */
    void partial_reverse(IdxT x, IdxT y) {
        if (x == y) return;
        IdxT t_next = prev_idx(x);
        IdxT t;
        do {
            t = t_next;
            t_next = prev_idx(t);
            link(x, t);
            x = t;
        } while (t != y);
    }

    /**
     * @brief vertex to idx
     *
     * @param ce
     *
     * @return
     */
    IdxT to_idx(const CycleEl &ce) const { return m_cycle_idx.get_idx(ce); }

    /**
     * @brief returns next idx in the cycle
     *
     * @param i
     *
     * @return
     */
    IdxT next_idx(IdxT i) const { return m_successor_map[i]; }

    /**
     * @brief returns previous idx
     *
     * @param i
     *
     * @return
     */
    IdxT prev_idx(IdxT i) const { return m_predecessor_map[i]; }

    /**
     * @brief idx to vertex
     *
     * @param i
     *
     * @return
     */
    const CycleEl &from_idx(IdxT i) const { return m_cycle_idx.get_val(i); }

    /**
     * @brief ads new element to cycle data structures
     *
     * @param el
     *
     * @return
     */
    IdxT add(const CycleEl &el) {
        m_predecessor_map.push_back(-1);
        m_successor_map.push_back(-1);
        return m_cycle_idx.add(el);
    }

    /// mapping from elements to indexes
    bimap<CycleEl, IdxT> m_cycle_idx;

    using SorsMap = std::vector<IdxT>;

    /// predecessors
    SorsMap m_predecessor_map;
    /// successors
    SorsMap m_successor_map;
};

/**
 * @brief this class adapts Simple cycle to start from last changed position
 *
 * @tparam CycleEl
 * @tparam IdxT
 */
template <typename CycleEl, typename IdxT = int>
class Simplecycle_start_from_last_change : public simple_cycle<CycleEl, IdxT> {
    using Base = simple_cycle<CycleEl, IdxT>;

  public:
    /**
     * @brief constructor
     *
     * @tparam Iter
     * @param b
     * @param e
     */
    template <typename Iter>
    Simplecycle_start_from_last_change(Iter b, Iter e)
        : Base(b, e), m_last_id(0) {}

    /**
     * @brief flip remembers last changed position
     *
     * @param begin
     * @param end
     */
    void flip(const CycleEl &begin, const CycleEl &end) {
        IdxT e1 = to_idx(begin);
        m_last_id = prev_idx(e1);
        Base::flip(begin, end);
    }

    /**
     * @brief vbegin starts from last flip position
     *
     * @return
     */
    typename Base::vertex_iterator vbegin() const {
        return Base::vbegin(from_idx(m_last_id));
    }

  private:
    IdxT m_last_id;
};

} // data_structures
} // paal

#endif // PAAL_SIMPLE_CYCLE_HPP
