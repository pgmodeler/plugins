//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file metric_on_idx.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-14
 */
#ifndef PAAL_METRIC_ON_IDX_HPP
#define PAAL_METRIC_ON_IDX_HPP

#include "paal/data_structures/bimap_traits.hpp"
#include "paal/data_structures/metric/basic_metrics.hpp"

namespace paal {
namespace data_structures {


struct read_values_tag{};
struct read_indexes_tag{};

/**
 * @brief This metric keeps inner metric and index.
 *        Given vertices are reindex and passed to inner metric.
 *
 * @tparam Metric
 * @tparam Bimap
 */
template <typename Metric, typename Bimap, typename Strategy = read_indexes_tag> class metric_on_idx {
    Metric m_metric;
    Bimap m_idx;
    using btraits = bimap_traits<typename std::decay<Bimap>::type>;

    auto read(typename btraits::Val v, read_values_tag) const -> decltype(m_idx.get_idx(v)) {
        return m_idx.get_idx(v);
    }

    auto read(typename btraits::Idx v, read_indexes_tag) const -> decltype(m_idx.get_val(v)) {
        return m_idx.get_val(v);
    }

    template <typename Vertex>
        auto read(Vertex && v) const -> decltype(this->read(v, Strategy())) {
        return read(v, Strategy{});
    }

  public:

    /**
     * @brief constructor
     *
     * @param m
     * @param idx
     */
    metric_on_idx(Metric m, Bimap idx)
        : m_metric(m), m_idx(idx) {}

    /**
     * @brief operator()
     *
     * @param i
     * @param j
     *
     * @return
     */
    template <typename Vertex>
    auto operator()(const Vertex & i, const Vertex & j) {
        return m_metric(read(i), read(j));
    }

    /**
     * @brief operator() const
     *
     * @param i
     * @param j
     *
     * @return
     */
    template <typename Vertex>
    auto operator()(const Vertex & i, const Vertex & j) const {
        return m_metric(read(i), read(j));
    }
};

/**
 * @brief make for metric_on_idx
 *
 * @tparam Metric
 * @tparam Bimap
 * @param m
 * @param b
 *
 * @return
 */
template <typename Strategy = read_indexes_tag, typename Metric, typename Bimap>
metric_on_idx<Metric, Bimap, Strategy> make_metric_on_idx(Metric && m,
                                                Bimap && b) {
    return metric_on_idx<Metric, Bimap, Strategy>(std::forward<Metric>(m), std::forward<Bimap>(b));
}


template <typename Metric, typename Bimap>
struct metric_traits<metric_on_idx<Metric, Bimap, read_indexes_tag>> :
public _metric_traits<metric_on_idx<Metric, Bimap, read_indexes_tag>,
       typename bimap_traits<typename std::decay<Bimap>::type>::Idx>
{};

template <typename Metric, typename Bimap>
struct metric_traits<metric_on_idx<Metric, Bimap, read_values_tag>> :
public _metric_traits<metric_on_idx<Metric, Bimap, read_values_tag>,
       typename bimap_traits<typename std::decay<Bimap>::type>::Val>
{};


} //! data_structures
} //! paal
#endif // PAAL_METRIC_ON_IDX_HPP
