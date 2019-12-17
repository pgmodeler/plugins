//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_components.hpp
 * @brief
 * @author Maciej Andrejczuk
 * @version 1.0
 * @date 2013-08-01
 */
#ifndef PAAL_STEINER_COMPONENTS_HPP
#define PAAL_STEINER_COMPONENTS_HPP

#include "paal/iterative_rounding/steiner_tree/steiner_component.hpp"

namespace paal {
namespace ir {

/**
 * Class responsible for storing all the components.
 * There are versions of components: ones with the same elements, but different
 * sink vertex.
 * All versions are stored in memory only once.
 */
template <typename Vertex, typename Dist> class steiner_components {
  public:
    steiner_components() : m_size(0) {}

    /**
     * Adds a new component in all its versions.
     */
    void add(const steiner_component<Vertex, Dist> &component) {
        for (int i = 0; i < (int)component.count_terminals(); ++i) {
            ++m_size;
            m_original_id.push_back(m_components.size());
            m_version.push_back(i);
        }
        m_components.push_back(component);
    }

    /**
     * Returns total number of components, each version counted separately.
     */
    int size() const { return m_size; }

    /**
     * Finds a component by its unique id.
     */
    const steiner_component<Vertex, Dist> &find(int id) const {
        assert(id >= 0 && id < m_size);
        return m_components[m_original_id[id]];
    }

    /**
     * Finds a version of component by its id.
     */
    int find_version(int id) const { return m_version[id]; }

    /**
     * Removes all data.
     */
    void clear() {
        m_components.clear();
        m_version.clear();
        m_original_id.clear();
        m_size = 0;
    }

  private:
    std::vector<steiner_component<Vertex, Dist>> m_components; // generated
                                                               // components
    std::vector<int> m_version;     // id -> version map
    std::vector<int> m_original_id; // id -> place in m_components vector
    int m_size; // m_version.size(), but not m_components.size()
};

} // ir
} // paal

#endif // PAAL_STEINER_COMPONENTS_HPP
