//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file singleton_iterator.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-02-27
 */
#ifndef PAAL_SINGLETON_ITERATOR_HPP
#define PAAL_SINGLETON_ITERATOR_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range/iterator_range.hpp>

namespace paal {
namespace utils {

/**
 * @brief Iterator to range containing single element.
 * @tparam Elem
 */
template <typename Elem>
class singleton_iterator : public boost::iterator_facade<
    singleton_iterator<Elem>, typename std::decay<Elem>::type,
    boost::forward_traversal_tag, Elem> {
    template <typename E>
    friend singleton_iterator<E> make_singleton_iterator_begin(E&&);
    template <typename E>
    friend singleton_iterator<E> make_singleton_iterator_end();

    /**
     * @brief private constructor. Use make_singleton_iterator_begin,
     *              make_singleton_iterator_end.
     *
     * @param elem
     */
    singleton_iterator(Elem elem) : m_elem(elem) {}

    friend class boost::iterator_core_access;

    void increment() { m_elem = boost::none; }

    bool equal(const singleton_iterator &other) const {
        // This doesn't need Elem to have (==) operator.
        return (!m_elem && !other.m_elem) || (m_elem && other.m_elem);
    }

    Elem dereference() const { return m_elem.get(); }

    boost::optional<Elem> m_elem;

    public:
       /**
        * @brief public constructor to satisfy the concept requirements. However
        * one should use make_singleton_iterator_begin,
        * make_singleton_iterator_end.
        */
       singleton_iterator() {}
};

/**
 * @brief function to create begin of singleton_iterator
 *
 * @tparam Elem
 * @param elem
 *
 * @return
 */
template <typename Elem>
singleton_iterator<Elem> make_singleton_iterator_begin(Elem&& elem) {
    return singleton_iterator<Elem>(std::forward<Elem>(elem));
}

/**
 * @brief function to create end of singleton_iterator
 *
 * @tparam Elem
 *
 * @return
 */
template <typename Elem>
singleton_iterator<Elem> make_singleton_iterator_end() {
    return singleton_iterator<Elem>();
}

/**
 * @brief function to create a singleton range
 *
 * @tparam Elem
 * @param elem
 *
 * @return
 */
template <typename Elem>
auto make_singleton_range(Elem&& elem)
-> decltype(boost::make_iterator_range(
    make_singleton_iterator_begin(std::forward<Elem>(elem)),
    make_singleton_iterator_end<Elem>()
)) {
    return boost::make_iterator_range(
        make_singleton_iterator_begin(std::forward<Elem>(elem)),
        make_singleton_iterator_end<Elem>()
    );
}

} //!utils
} //!paal

#endif // PAAL_SINGLETON_ITERATOR_HPP
