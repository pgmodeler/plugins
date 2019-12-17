//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file combine_iterator.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#include "paal/utils/type_functions.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/empty.hpp>

#ifndef PAAL_COMBINE_ITERATOR_HPP
#define PAAL_COMBINE_ITERATOR_HPP

namespace paal {
namespace data_structures {
//TODO change name to product
/**
 * @brief class representing set of ranges with two operation next and call
 *
 * @tparam Ranges
 */
template <typename... Ranges> class combine_iterator_engine;

/**
 * @class combine_iterator_engine
 * @brief actual implementation
 *
 * @tparam Range
 * @tparam RangesRest
 */
template <typename Range, typename... RangesRest>
class combine_iterator_engine<
    Range, RangesRest...> : private combine_iterator_engine<RangesRest...> {

  public:
    using base = combine_iterator_engine<RangesRest...>;
    using Iterator = typename boost::range_iterator<Range>::type;

    /**
     * @brief constructor
     *
     * @param range
     * @param rest
     */
    combine_iterator_engine(Range &range, RangesRest &... rest)
        : base(rest...), m_begin(std::begin(range)), m_curr(std::begin(range)),
          m_end(std::end(range)) {}

    combine_iterator_engine() = default;

    /**
     * @brief move iterators to the next position
     *
     * @return
     */
    bool next() {
        if (!base::next()) {
            ++m_curr;
            if (m_curr == m_end) {
                m_curr = m_begin;
                return false;
            }
        }
        return true;
    }

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
    auto call(F f, Args &&... args)->decltype(std::declval<base>().call(
        std::move(f), std::forward<Args>(args)..., *std::declval<Iterator>())) {
        return base::call(std::move(f), std::forward<Args>(args)..., *m_curr);
    }

    /**
     * @brief operator==
     *
     * @param left
     * @param right
     *
     * @return
     */
    friend bool operator==(const combine_iterator_engine &left,
                           const combine_iterator_engine &right) {
        return left.m_begin == right.m_begin && left.m_end == right.m_end &&
               left.m_curr == right.m_curr &&
               static_cast<base>(left) == static_cast<base>(right);
    }

  private:
    Iterator m_begin;
    Iterator m_curr;
    Iterator m_end;
};

/**
 * @brief specialization for empty ranges lists
 */
template <> class combine_iterator_engine<> {
  public:
    /**
     * @brief no next configuration
     *
     * @return
     */
    bool next() { return false; }

    /**
     * @brief actually  calls function f
     *
     * @tparam F
     * @tparam Args
     * @param f
     * @param args
     *
     * @return
     */
    template <typename F, typename... Args>
    auto call(F f, Args &&... args)->decltype(f(std::forward<Args>(args)...)) {
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
    friend bool operator==(const combine_iterator_engine &left,
                           const combine_iterator_engine &right) {
        return true;
    }
};

namespace detail {
// TODO can you do this without alias???
template <typename T> using rem_ref = typename std::remove_reference<T>::type;
}

/**
 * @brief make for combine_iterator_engine
 *
 * @tparam Ranges
 * @param ranges
 *
 * @return
 */
template <typename... Ranges>
combine_iterator_engine<detail::rem_ref<Ranges>...>
make_combine_iterator_engine(Ranges &&... ranges) {
    // see comments in make_combine_iterator
    return combine_iterator_engine<detail::rem_ref<Ranges>...>{ ranges... };
}

/**
 * @brief combine_iterator iterates through all combinations of values from
 * given ranges
 *        and returns them joined together using given Joiner
 *
 * @tparam Joiner
 * @tparam Ranges
 */
template <typename Joiner, typename... Ranges>
class combine_iterator : public boost::iterator_facade<
    combine_iterator<Joiner, Ranges...>,
    puretype(combine_iterator_engine<Ranges...>().call(std::declval<Joiner>())),
    boost::forward_traversal_tag // TODO this should be minimal tag of the
                                 // ranges
    ,
    decltype(
        combine_iterator_engine<Ranges...>().call(std::declval<Joiner>()))> {
  public:
    /**
     * @brief constructor
     *
     * @param joiner
     * @param ranges
     */
    combine_iterator(Joiner joiner, Ranges &... ranges)
        : m_joiner(joiner), m_iterator_engine(ranges...),
          m_end(sizeof...(Ranges) ? is_empty(ranges...) : true) {}

    /**
     * @brief default constructor represents end of the range
     */
    combine_iterator() : m_end(true) {};

  private:
    /**
     * @brief returns true if at least one of given ranges is empty
     *
     * @tparam Range
     * @tparam RangesRest
     * @param range
     * @param rest
     *
     * @return
     */
    template <typename Range, typename... RangesRest>
    bool is_empty(const Range &range, const RangesRest &... rest) {
        if (boost::empty(range)) {
            return true;
        } else {
            return is_empty(rest...);
        }
    }

    /**
     * @brief boundary case for is_empty
     *
     * @return
     */
    bool is_empty() { return false; }

    using ref = decltype(
        combine_iterator_engine<Ranges...>().call(std::declval<Joiner>()));

    friend class boost::iterator_core_access;

    /**
     * @brief increments iterator
     */
    void increment() {
        if (!m_iterator_engine.next()) {
            m_end = true;
        }
    }

    /**
     * @brief equal function
     *
     * @param other
     *
     * @return
     */
    bool equal(combine_iterator const &other) const {
        return this->m_end == other.m_end &&
               (this->m_end ||
                this->m_iterator_engine == other.m_iterator_engine);
    }

    /**
     * @brief dereference
     *
     * @return
     */
    ref dereference() const { return m_iterator_engine.call(m_joiner); }

    Joiner m_joiner;
    mutable combine_iterator_engine<Ranges...> m_iterator_engine;
    bool m_end;
};

/**
 * @brief make for combine_iterator
 *
 * @tparam Joiner
 * @tparam Ranges
 * @param joiner
 * @param ranges
 *
 * @return
 */
template <typename Joiner, typename... Ranges>
combine_iterator<Joiner, detail::rem_ref<Ranges>...>
make_combine_iterator(Joiner joiner, Ranges &&... ranges) {
    // we do not forward the ranges, because combine_iterator expects lvalues
    // we Use Ranges && because, we'd like to cover const/nonconst cases
    return combine_iterator<Joiner, detail::rem_ref<Ranges>...>{ joiner,
                                                                 ranges... };
}

} // data_structures
} // paal

#endif // PAAL_COMBINE_ITERATOR_HPP
