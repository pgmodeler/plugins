//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file 2_local_search_solution_adapter.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
#ifndef PAAL_2_LOCAL_SEARCH_SOLUTION_ADAPTER_HPP
#define PAAL_2_LOCAL_SEARCH_SOLUTION_ADAPTER_HPP

#include "paal/data_structures/vertex_to_edge_iterator.hpp"

namespace paal {
namespace local_search {

/**
 * @brief adapts cycle to have begin and end pointing to edge collection
 *
 * @tparam Cycle
 */
template <typename Cycle> class two_local_search_adapter {
  public:
    typedef typename Cycle::vertex_iterator vertex_iterator;
    typedef data_structures::vertex_to_edge_iterator<vertex_iterator> Iterator;

    /**
     * @brief constructor
     *
     * @param cm
     */
    two_local_search_adapter(Cycle &cm) : m_cycle(cm) {}

    /**
     * @brief Edges begin
     *
     * @return
     */
    Iterator begin() const {
        return data_structures::make_vertex_to_edge_iterator(m_cycle.vbegin(),
                                                             m_cycle.vend());
    }

    /**
     * @brief Edges end
     *
     * @return
     */
    Iterator end() const {
        auto end = m_cycle.vend();
        return data_structures::make_vertex_to_edge_iterator(end, end);
    }

    /**
     * @brief gets adopted cycle
     *
     * @return
     */
    Cycle &get_cycle() { return m_cycle; }

    /**
     * @brief gets adopted cycle const version
     *
     * @return
     */
    const Cycle &get_cycle() const { return m_cycle; }

  private:

    Cycle &m_cycle;
};

} // local_search
} // paal

#endif // PAAL_2_LOCAL_SEARCH_SOLUTION_ADAPTER_HPP
