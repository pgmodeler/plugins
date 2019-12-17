/**
 * @file stack.hpp
 * @brief stack that doesn't call destructor on pop
 * @author Piotr Smulewicz, Robert Rosołek
 * @version 1.0
 * @date 2014-08-12
 */
#ifndef PAAL_STACK_HPP
#define PAAL_STACK_HPP

#include <vector>

namespace paal {
namespace data_structures {

/// Stack
template <class T> class stack {
    std::vector<T> m_v;
    std::size_t m_size;
  public:
    /// constructor
    stack() : m_v(std::vector<T>()), m_size(0) {}
    /// push
    void push() {
        if (++m_size > m_v.size()) m_v.resize(m_size);
    }
    /// pop doesn't call destructor
    void pop() { --m_size; }
    /// top
    const T &top() const { return m_v[m_size - 1]; }
    /// top
    T &top() { return m_v[m_size - 1]; }
    /// empty
    bool empty() const { return m_size == 0; }
    /// size
    std::size_t size() const { return m_size; }
};

} // !data_structures
} // !paal

#endif /* PAAL_STACK_HPP */
