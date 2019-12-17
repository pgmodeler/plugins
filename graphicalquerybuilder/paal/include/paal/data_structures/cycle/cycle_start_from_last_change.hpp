//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file cycle_start_from_last_change.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-26
 */
#ifndef PAAL_CYCLE_START_FROM_LAST_CHANGE_HPP
#define PAAL_CYCLE_START_FROM_LAST_CHANGE_HPP

#include "cycle_traits.hpp"

namespace paal {
namespace data_structures {

/**
 * @brief adopts any cycle to start (vbegin) i place of the last change(flip)
 *
 * @tparam Cycle
 */
template <typename Cycle> class cycle_start_from_last_change {
  public:
    typedef typename cycle_traits<Cycle>::CycleElem CycleElem;
    typedef typename cycle_traits<Cycle>::vertex_iterator vertex_iterator;

    /**
     * @brief constructor
     *
     * @param c
     */
    cycle_start_from_last_change(Cycle &c)
        : m_cycle(c), m_element(*c.vbegin()) {}

    /**
     * @brief flip stores place of this flip
     *
     * @param begin
     * @param end
     */
    void flip(const CycleElem &begin, const CycleElem &end) {
        m_element = end;
        m_cycle.flip(begin, end);
    }

    /**
     * @brief vbegin starts from last flip
     *
     * @return
     */
    vertex_iterator vbegin() const { return m_cycle.vbegin(m_element); }

    /**
     * @brief vbegin starts from ce
     *
     * @param ce
     *
     * @return
     */
    vertex_iterator vbegin(const CycleElem &ce) const {
        return m_cycle.vbegin(ce);
    }

    /**
     * @brief vertices end
     *
     * @return
     */
    vertex_iterator vend() const { return m_cycle.vend(); }

    /**
     * @brief cycle getter
     *
     * @return
     */
    Cycle &get_cycle() { return m_cycle; }

    /**
     * @brief cycle getter const version
     *
     * @return
     */
    const Cycle &get_cycle() const { return m_cycle; }

  private:
    Cycle &m_cycle;
    CycleElem m_element;
};
}
}

#endif // PAAL_CYCLE_START_FROM_LAST_CHANGE_HPP
