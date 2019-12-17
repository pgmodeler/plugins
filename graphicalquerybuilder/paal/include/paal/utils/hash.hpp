//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file hash.hpp
 * @brief
 * @author Piotr Wygocki, Andrzej Pacuk
 * @version 1.0
 * @date 2013-12-19
 */
#ifndef PAAL_HASH_HPP
#define PAAL_HASH_HPP

#include <boost/functional/hash.hpp>
#include <boost/graph/graph_traits.hpp>

#include <algorithm>
#include <cstddef>

namespace paal {

/**
 * @brief hash for edge_descriptor in bgl, undirected version
 *
 * @tparam Graph
 * @tparam Enable
 */
template <typename Graph, class Enable = void> struct edge_hash {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    /**
     * @brief operator()
     *
     * @param e
     *
     * @return
     */
    std::size_t operator()(const Edge &e) const {
        std::size_t hash = 0;
        boost::hash_combine(hash, std::min(e.m_source, e.m_target));
        boost::hash_combine(hash, std::max(e.m_source, e.m_target));
        return hash;
    }
};
/**
 * @brief hash for edge_descriptor in bgl, directed version
 *
 * @tparam Graph
 * @tparam Enable
 */

template <typename Graph>
struct edge_hash<Graph,
                 typename std::enable_if<std::is_same<
                     typename boost::graph_traits<Graph>::directed_category,
                     boost::directed_tag>::value>::type> {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    /**
     * @brief operator()
     *
     * @param e
     *
     * @return
     */
    std::size_t operator()(const Edge &e) const {
        std::size_t hash = 0;
        boost::hash_combine(hash, e.m_source);
        boost::hash_combine(hash, e.m_target);
        return hash;
    }
};

/**
 * @brief Functor that can be used as a std::unordered_map/set hash param,
 * when key is a range.
 * copied from:
 * http://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
 *
 * @tparam Range
 */
struct range_hash {

    /**
     * @brief
     *
     * @param range
     *
     * @return
     */
    template <typename Range>
    std::size_t operator()(Range && range) const {
        return boost::hash_range(std::begin(range), std::end(range));
    }
};

} //! paal

#endif // PAAL_HASH_HPP
