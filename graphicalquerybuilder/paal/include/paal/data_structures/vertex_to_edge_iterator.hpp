//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file vertex_to_edge_iterator.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-20
 */
#ifndef PAAL_VERTEX_TO_EDGE_ITERATOR_HPP
#define PAAL_VERTEX_TO_EDGE_ITERATOR_HPP

#include "paal/utils/type_functions.hpp"

namespace paal {
namespace data_structures {

// TODO use boost:::iterator_fascade
/**
 * @class vertex_to_edge_iterator
 * @brief transforms collection to collection of pairs consecutive elements of
 * the input collection.
 *      The last element and the first element are considered consecutive.
 *
 * @tparam vertex_iterator
 */
template <typename vertex_iterator> class vertex_to_edge_iterator {
  public:
    typedef typename std::iterator_traits<vertex_iterator>::value_type Vertex;
    typedef std::pair<Vertex, Vertex> Edge;

    typedef std::forward_iterator_tag iterator_category;
    typedef Edge value_type;
    typedef ptrdiff_t difference_type;
    typedef Edge *pointer;
    typedef const Edge &reference;

    /**
     * @brief constructor
     *
     * @param b
     * @param e
     */
    vertex_to_edge_iterator(vertex_iterator b, vertex_iterator e)
        : m_idx(b), m_begin(b), m_end(e) {
        move_curr();
    }

    vertex_to_edge_iterator() = default;

    /**
     * @brief operator++ post increment
     *
     * @return
     */
    vertex_to_edge_iterator &operator++() {
        ++m_idx;
        move_curr();

        return *this;
    }

    /**
     * @brief operator++ pre increment
     *
     * @return
     */
    vertex_to_edge_iterator operator++(int) {
        vertex_to_edge_iterator i(*this);
        operator++();
        return i;
    }

    /**
     * @brief operator !=
     *
     * @param ei
     *
     * @return
     */
    bool operator!=(vertex_to_edge_iterator ei) const {
        return !operator==(ei);
    }

    /**
     * @brief operator==
     *
     * @param ei
     *
     * @return
     */
    bool operator==(vertex_to_edge_iterator ei) const {
        return m_idx == ei.m_idx;
    }

    /**
     * @brief operator->
     *
     * @return
     */
    const Edge *const operator->() const { return &m_curr; }

    /**
     * @brief operator*
     *
     * @return
     */
    const Edge &operator*() const { return m_curr; }

  private:
    /**
     * @brief moves iterators to next position
     */
    void move_curr() {
        if (m_idx != m_end) {
            m_curr.first = *m_idx;
            vertex_iterator next = m_idx;
            ++next;
            if (next == m_end) {
                m_curr.second = *m_begin;
            } else {
                m_curr.second = *next;
            }
        }
    }

    vertex_iterator m_idx;
    vertex_iterator m_begin;
    vertex_iterator m_end;
    Edge m_curr;
};

/**
 * @brief make for vertex_to_edge_iterator
 *
 * @tparam vertex_iterator
 * @param b
 * @param e
 *
 * @return
 */
template <typename vertex_iterator>
vertex_to_edge_iterator<vertex_iterator>
make_vertex_to_edge_iterator(vertex_iterator b, vertex_iterator e) {
    return vertex_to_edge_iterator<vertex_iterator>(b, e);
}

/**
 * @brief make for vertex_to_edge_iterator form Vertex iterator pair
 *
 * @tparam vertex_iterator
 * @param r
 *
 * @return
 */
template <typename vertex_iterator>
vertex_to_edge_iterator<vertex_iterator>
make_vertex_to_edge_iterator(std::pair<vertex_iterator, vertex_iterator> r) {
    return vertex_to_edge_iterator<vertex_iterator>(r.first, r.second);
}

} // data_structures
} // paal

#endif // PAAL_VERTEX_TO_EDGE_ITERATOR_HPP
