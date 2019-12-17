//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file subset_iterator.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#include "paal/utils/type_functions.hpp"
#include "paal/utils/make_tuple.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/iterator_range.hpp>

#ifndef PAAL_SUBSET_ITERATOR_HPP
#define PAAL_SUBSET_ITERATOR_HPP

namespace paal {
namespace data_structures {
/**
 * @tparam Iterator
 * @tparam k
 */
template <int k, typename Iterator>
class subsets_iterator_engine
    : private subsets_iterator_engine<k - 1, Iterator> {
  protected:
    /**
     * @brief current being
     *
     * @return
     */
    Iterator get_begin() { return m_begin; }

    /**
     * @brief end is stored in the subsets_iterator_engine<0>
     *
     * @return
     */
    Iterator get_end() { return base::get_end(); }

    /**
     * @brief sets all iterators to m_end
     */
    void set_to_end() {
        m_begin = get_end();
        base::set_to_end();
    }

  public:
    using base = subsets_iterator_engine<k - 1, Iterator>;

    /**
     * @brief constructor
     *
     * @param begin
     * @param end
     */
    subsets_iterator_engine(Iterator begin, Iterator end) : base(begin, end) {
        if (k == 1) {
            m_begin = begin;
        } else {
            auto baseBegin = base::get_begin();
            if (baseBegin != end) {
                m_begin = ++baseBegin;
                if (m_begin == end) {
                    // when we are at the end all iterators are set to m_end
                    base::set_to_end();
                }
            } else {
                // when we are at the end all iterators are set to m_end
                set_to_end();
            }
        }
    }

    subsets_iterator_engine() = default;

    /**
     * @brief sets next configuration of iterators, pointing to next subset
     *
     * @return
     */
    bool next() {
        ++m_begin;
        while (m_begin == get_end()) {
            if (base::next()) {
                m_begin = base::get_begin();
                if (m_begin == get_end()) {
                    // when we are at the end all iterators are set to m_end
                    base::set_to_end();
                    return false;
                }
                ++m_begin;
            } else {
                return false;
            }
        }
        return true;
    }

    // TODO copy paste (combine_iterator)
    /**
     * @brief calls arbitrary function f on (*m_curr)...
     *
     * @tparam F
     * @tparam Args
     * @param f
     * @param args
     *
     * @return
     */
    template <typename F, typename... Args>
    auto call(F f, Args &&... args) const->decltype(std::declval<base>().call(
        std::move(f), std::forward<Args>(args)..., *std::declval<Iterator>())) {
        return base::call(std::move(f), *m_begin, std::forward<Args>(args)...);
    }

    /**
     * @brief operator==
     *
     * @param left
     * @param right
     *
     * @return
     */
    friend bool operator==(const subsets_iterator_engine &left,
                           const subsets_iterator_engine &right) {
        return left.m_begin == right.m_begin &&
               static_cast<base>(left) == static_cast<base>(right);
    }

  private:
    Iterator m_begin;
};

/**
 * @brief specialization for k==0 for boundary cases.
 *      This class stores iterator pointing to the end of the input collection
 *
 * @tparam Iterator
 */
template <typename Iterator> class subsets_iterator_engine<0, Iterator> {
  protected:
    /**
     * @brief constructor
     *
     * @param begin
     * @param end
     */
    subsets_iterator_engine(Iterator begin, Iterator end) : m_end(end) {}

    subsets_iterator_engine() = default;

    /**
     * @brief get_begin, fake returns m_end
     *
     * @return
     */
    Iterator get_begin() { return m_end; }

    /**
     * @brief get_end, returns end of the input collection
     *
     * @return
     */
    Iterator get_end() { return m_end; }

    /**
     * @brief boundary case, does nothing
     */
    void set_to_end() {}

  public:
    /**
     * @brief boundary case, does nothing
     *
     * @return
     */
    bool next() const { return false; }

    /**
     * @brief actually calls f for given arguments
     *
     * @tparam F
     * @tparam Args
     * @param f
     * @param args
     *
     * @return
     */
    template <typename F, typename... Args>
    auto call(F f,
              Args &&... args) const->decltype(f(std::forward<Args>(args)...)) {
        return f(std::forward<Args>(args)...);
    }

    /**
     * @brief operator==, always true
     *
     * @param left
     * @param right
     *
     * @return
     */
    friend bool operator==(const subsets_iterator_engine &left,
                           const subsets_iterator_engine &right) {
        return true;
    }

  private:
    Iterator m_end;
};

/**
 * @brief make for subsets_iterator_engine
 *
 * @tparam k
 * @tparam Iterator
 * @param b
 * @param e
 *
 * @return
 */
template <int k, typename Iterator>
subsets_iterator_engine<k, Iterator> make_subsets_iterator_engine(Iterator b,
                                                                  Iterator e) {
    return subsets_iterator_engine<k, Iterator>(b, e);
}

/**
 * @class subsets_iterator
 * @brief Iterator to all k-subsets of given collection.
 *
 * @tparam Iterator
 * @tparam k
 * @tparam Joiner
 */
template <int k, typename Iterator, typename Joiner = make_tuple>
class subsets_iterator : public boost::iterator_facade<
    subsets_iterator<k, Iterator, Joiner>,
    puretype(
        (subsets_iterator_engine<k, Iterator>().call(std::declval<Joiner>())))
    //                            , typename
    // std::iterator_traits<Iterator>::iterator_category //TODO above forward
    // tags are not yet implemented
    ,
    typename boost::forward_traversal_tag,
    decltype(
        subsets_iterator_engine<k, Iterator>().call(std::declval<Joiner>()))> {
  public:
    /**
     * @brief constructor
     *
     * @param begin
     * @param end
     * @param joiner
     */
    subsets_iterator(Iterator begin, Iterator end, Joiner joiner = Joiner{})
        : m_joiner(joiner), m_iterator_engine(begin, end) {}

    /**
     * @brief default constructor represents end of the range
     */
    subsets_iterator() = default;

  private:

    /**
     * @brief reference type of the iterator
     */
    using ref = decltype(
        subsets_iterator_engine<k, Iterator>().call(std::declval<Joiner>()));

    friend class boost::iterator_core_access;

    /**
     * @brief increments iterator
     */
    void increment() { m_iterator_engine.next(); }

    /**
     * @brief equal function
     *
     * @param other
     *
     * @return
     */
    bool equal(subsets_iterator const &other) const {
        return this->m_iterator_engine == other.m_iterator_engine;
    }

    /**
     * @brief dereference
     *
     * @return
     */
    ref dereference() const { return m_iterator_engine.call(m_joiner); }
    // TODO add random access support

    Joiner m_joiner;
    subsets_iterator_engine<k, Iterator> m_iterator_engine;
};

/**
 * @brief make for subsets_iterator
 *
 * @tparam Iterator
 * @tparam k
 * @tparam Joiner
 * @param b
 * @param e
 * @param joiner
 *
 * @return
 */
//TODO change name to subset_range()
template <int k, typename Iterator, typename Joiner = make_tuple>
boost::iterator_range<subsets_iterator<k, Iterator, Joiner>>
make_subsets_iterator_range(Iterator b, Iterator e, Joiner joiner = Joiner{}) {
    typedef subsets_iterator<k, Iterator, Joiner> SI;
    return boost::make_iterator_range(SI(b, e, joiner), SI(e, e, joiner));
}

/**
 * @brief
 *
 * @tparam k
 * @tparam Range
 * @tparam Joiner
 * @param range
 * @param joiner
 *
 * @return
 */
template <int k, typename Range, typename Joiner = make_tuple>
auto make_subsets_iterator_range(const Range & range, Joiner joiner = Joiner{}) {
    return make_subsets_iterator_range<k>(std::begin(range), std::end(range), std::move(joiner));
}

} // data_structures
} // paal

#endif // PAAL_SUBSET_ITERATOR_HPP
