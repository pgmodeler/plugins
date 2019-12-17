//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file collection_starts_from_last_change.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-11
 */
#ifndef PAAL_COLLECTION_STARTS_FROM_LAST_CHANGE_HPP
#define PAAL_COLLECTION_STARTS_FROM_LAST_CHANGE_HPP

#include <boost/range/join.hpp>

#include <unordered_map>

namespace paal {
namespace data_structures {

/**
 * @brief this collection stores some range and expose set_last_change function
 *        each time begin and end is called this class returns range which
 * starts from last change place
 *
 * @tparam Iterator
 * @tparam hash
 */
template <typename Iterator,
          typename hash =
              std::hash<typename std::iterator_traits<Iterator>::value_type>>
class collection_starts_from_last_change {
    typedef typename std::iterator_traits<Iterator>::value_type Element;
    typedef std::unordered_map<Element, Iterator, hash> ElemToIter;
    typedef std::pair<Iterator, Iterator> Range;
    typedef boost::joined_range<Range, Range> JoinedRange;
    typedef typename boost::range_iterator<JoinedRange>::type JoinedIterator;

  public:
    typedef JoinedIterator ResultIterator;

    collection_starts_from_last_change() = default;

    /**
     * @brief constructor
     *
     * @param begin
     * @param end
     */
    collection_starts_from_last_change(Iterator begin, Iterator end)
        : m_begin(begin), m_end(end), m_new_begin(m_begin) {
        assert(m_begin != m_end);
        for (auto i = m_begin; i != m_end; ++i) {
            bool b = m_elem_to_iter.emplace(*i, i).second;
            assert(b);
        }
    }

    /**
     * @brief one can set the place of the last change (future start position of
     * the range)
     *
     * @param el
     */
    void set_last_change(const Element &el) {
        auto i = m_elem_to_iter.find(el);
        assert(i != m_elem_to_iter.end());
        m_new_begin = i->second;
    }

    /**
     * @brief begin
     *
     * @return
     */
    JoinedIterator begin() { return std::begin(get_range()); }

    /**
     * @brief end
     *
     * @return
     */
    JoinedIterator end() { return std::end(get_range()); }

  private:
    /**
     * @brief gets range
     *
     * @return
     */
    JoinedRange get_range() {
        Range r1 = std::make_pair(m_new_begin, m_end);
        Range r2 = std::make_pair(m_begin, m_new_begin);
        return boost::join(r1, r2);
    }

    Iterator m_begin;
    Iterator m_end;
    Iterator m_new_begin;
    ElemToIter m_elem_to_iter;
};
}
}

#endif // PAAL_COLLECTION_STARTS_FROM_LAST_CHANGE_HPP
