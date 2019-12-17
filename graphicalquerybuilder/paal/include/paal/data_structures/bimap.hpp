//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//               2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file bimap.hpp
 * @brief
 * @author Piotr Wygocki, Piotr Smulewicz
 * @version 1.1
 * @date 2013-09-12
 */
#ifndef PAAL_BIMAP_HPP
#define PAAL_BIMAP_HPP

#include "paal/data_structures/bimap_traits.hpp"
#include "paal/utils/irange.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/functional/hash.hpp>

#include <unordered_map>

namespace paal {
namespace data_structures {

/**
 * @class bimap_mic
 * @brief the same as Bimap, but implemented using boost::multi_index_container.
 * Unfortunately slower
 *
 * @tparam T
 * @tparam Idx
 */
template <typename T, typename Idx = int> class bimap_mic {
  public:

    bimap_mic() = default;

    /**
     * @brief constructor
     *
     * @tparam Range
     * @param range
     */
    template <typename Range> bimap_mic(Range && range) {
        std::size_t s = boost::distance(range);
        m_index.reserve(s);
        for (const T &t : range) {
            add(t);
        }
    }

    /**
     * @brief get_idx on element t
     *
     * @param t
     *
     * @return
     */
    Idx get_idx(const T &t) const {
        auto const &idx = m_index.template get<1>();
        return m_index.template project<0>(idx.find(t)) - m_index.begin();
    }

    /**
     * @brief get element on index i
     *
     * @param i
     *
     * @return
     */
    const T &get_val(Idx i) const {
#ifdef NDEBUG
        return m_index[i];
#else
        return m_index.at(i);
#endif
    }

    /**
     * @brief number of elements
     *
     * @return
     */
    std::size_t size() const { return m_index.size(); }

    /**
     * @brief adds alement to bimap
     *
     * @param t
     *
     * @return
     */
    Idx add(const T &t) {
        m_index.push_back(t);
        return m_index.size() - 1;
    }

  private:
    typedef boost::multi_index_container<
        T, boost::multi_index::indexed_by<boost::multi_index::random_access<>,
                                          boost::multi_index::hashed_unique<
                                              boost::multi_index::identity<T>>>>
        bm_type;
    bm_type m_index;
};

// minor TODO write specification when T is integral (copy instead of reference)
/**
 * @class bimap
 * @brief implements both sides mapping from the collection to
 * (0,size(collection)) interval.
 *
 * @tparam T
 * @tparam Idx
 */
template <typename T, typename Idx = int> class bimap {
    typedef std::unordered_map<T, Idx, boost::hash<T>> TToID;

  public:
    typedef typename TToID::const_iterator Iterator;

    bimap() = default;

    /**
     * @brief constructor
     *
     * @tparam Range
     * @param range
     */
    template <typename Range> bimap(Range && range) {
        std::size_t s = boost::distance(range);
        m_id_to_t.reserve(s);
        m_t_to_id.reserve(s);
        for (const T &t : range) {
            add(t);
        }
    }

    /**
     * @brief gets index of element t
     *
     * @param t
     *
     * @return
     */
    Idx get_idx(const T &t) const {
        auto iter = m_t_to_id.find(t);
        assert(iter != m_t_to_id.end());
        return iter->second;
    }

    /**
     * @brief get value for index i
     *
     * @param i
     *
     * @return
     */
    const T &get_val(Idx i) const {
#ifdef NDEBUG
        return m_id_to_t[i];
#else
        return m_id_to_t.at(i);
#endif
    }

    /**
     * @brief number of elements
     *
     * @return
     */
    std::size_t size() const { return m_id_to_t.size(); }

    /**
     * @brief adds element to collection
     *
     * @param t
     *
     * @return
     */
    Idx add(const T &t) {
        assert(m_t_to_id.find(t) == m_t_to_id.end());
        Idx idx = size();
        m_t_to_id[t] = idx;
        m_id_to_t.push_back(t);
        return idx;
    }

    /**
     * @brief get range of all element, index pairs
     *
     * @return
     */
    std::pair<Iterator, Iterator> get_range() const {
        return std::make_pair(m_t_to_id.begin(), m_t_to_id.end());
    }

  protected:
    /// mapping from id to element
    std::vector<T> m_id_to_t;
    /// mapping from elements to ids
    TToID m_t_to_id;
};

/**
 * @brief this maps support erasing elements, Alert inefficient!!
 *
 * @tparam T
 * @tparam Idx
 */
template <typename T, typename Idx = int>
class eraseable_bimap : public bimap<T, Idx> {
    typedef bimap<T, Idx> base;
    using base::m_t_to_id;
    using base::m_id_to_t;

  public:
    /**
     * @brief erases element (takes linear time)
     *
     * @param t
     */
    void erase(const T &t) {
        auto iter = m_t_to_id.find(t);
        assert(iter != m_t_to_id.end());
        Idx idx = iter->second;
        m_t_to_id.erase(iter);
        m_id_to_t.erase(m_id_to_t.begin() + idx);

        for (int i : irange(idx, Idx(m_id_to_t.size()))) {
            assert(m_t_to_id.at(m_id_to_t[i]) == i + 1);
            m_t_to_id[m_id_to_t[i]] = i;
        }
    }
};

/**
 * @brief in this bimap we know that elements forms permutation
 *        this allows optimization
 *
 * @tparam T
 * @tparam Idx
 */
template <typename T, typename Idx = int> class bimap_of_consecutive {
    // TODO maybe it should be passed but only on debug
    static const Idx INVALID_IDX = -1;

  public:
    static_assert(std::is_integral<T>::value, "Type T has to be integral");
    bimap_of_consecutive() = default;

    /**
     * @brief constructor
     *
     * @tparam Iter
     * @param b
     * @param e
     */
    template <typename Iter> bimap_of_consecutive(Iter b, Iter e) {
        if (b == e) return;

        std::size_t size = std::distance(b, e);
        m_id_to_t.resize(size);
        std::copy(b, e, m_id_to_t.begin());

        m_t_to_id.resize(size, INVALID_IDX);
        rank(m_id_to_t, m_t_to_id, INVALID_IDX);
    }

    /**
     * @brief gets index of element t
     *
     * @param t
     *
     * @return
     */
    Idx get_idx(const T &t) const { return m_t_to_id[t]; }

    /**
     * @brief gets value for index i
     *
     * @param i
     *
     * @return
     */
    const T &get_val(Idx i) const { return m_id_to_t[i]; }

    /**
     * @brief number of elements
     *
     * @return
     */
    std::size_t size() const { return m_id_to_t.size(); }

  private:
    std::vector<T> m_id_to_t;
    std::vector<Idx> m_t_to_id;
};

/**
 * @brief traits specialization for Bimap
 *
 * @tparam ValT
 * @tparam IdxT
 */
template <typename ValT, typename IdxT> struct bimap_traits<bimap<ValT, IdxT>> {
    typedef ValT Val;
    typedef IdxT Idx;
};

/**
 * @brief traits specialization for eraseable_bimap
 *
 * @tparam ValT
 * @tparam IdxT
 */
template <typename ValT, typename IdxT>
struct bimap_traits<eraseable_bimap<ValT, IdxT>> {
    typedef ValT Val;
    typedef IdxT Idx;
};

/**
 * @brief traits specialization for bimap_of_consecutive
 *
 * @tparam ValT
 * @tparam IdxT
 */
template <typename ValT, typename IdxT>
struct bimap_traits<bimap_of_consecutive<ValT, IdxT>> {
    typedef ValT Val;
    typedef IdxT Idx;
};

/**
 * @brief traits specialization for bimap_mic
 *
 * @tparam ValT
 * @tparam IdxT
 */
template <typename ValT, typename IdxT>
struct bimap_traits<bimap_mic<ValT, IdxT>> {
    typedef ValT Val;
    typedef IdxT Idx;
};

/**
 * @brief computes rank i.e. index of element in range
 *
 * @tparam T
 * @tparam Idx
 * @param m_id_to_t
 * @param m_t_to_id
 * @param INVALID_IDX
 */
template <typename T, typename Idx = int>
void rank(std::vector<T> const &m_id_to_t, std::vector<Idx> &m_t_to_id,
          int INVALID_IDX = 0) {
    static_assert(std::is_integral<T>::value, "Type T has to be integral");
    unsigned long size = m_t_to_id.size();
    for (auto i : irange(size)) {
        Idx &idx = m_t_to_id[m_id_to_t[i]];
        assert(m_id_to_t[i] < int(size) && idx == INVALID_IDX);
        idx = i;
    }
}

} //! data_structures
} //! paal
#endif // PAAL_BIMAP_HPP
