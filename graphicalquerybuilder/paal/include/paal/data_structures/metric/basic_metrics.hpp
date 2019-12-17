//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file basic_metrics.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-15
 */
#ifndef PAAL_BASIC_METRICS_HPP
#define PAAL_BASIC_METRICS_HPP

#include "metric_traits.hpp"

#include <boost/multi_array.hpp>
#include <boost/range/iterator_range.hpp>

#include <array>

namespace paal {
namespace data_structures {

/**
 * @class rectangle_array_metric
 * @brief \ref metric implementation on 2 dimensional array
 *        distance calls on this metric are valid opnly when x < N and y  < M
 *        (N and M given in the constructor)
 *        when we know that only certain calls occurs it might be worthwhile to
 * use this metric
 *
 * @tparam DistanceTypeParam
 */
template <typename DistanceTypeParam> class rectangle_array_metric {
  public:
    typedef DistanceTypeParam DistanceType;
    typedef int VertexType;
    /**
     * @brief constructor
     *
     * @param N
     * @param M
     */
    rectangle_array_metric(int N = 0, int M = 0)
        : m_matrix(boost::extents[N][M]) {}

    /**
     * @brief operator(), valid only when v < N and w < M
     *
     * @param v
     * @param w
     *
     * @return
     */
    DistanceType operator()(const VertexType &v, const VertexType &w) const {
        return m_matrix[v][w];
    }

    /**
     * @brief operator(), valid only when v < N and w < M, nonconst version
     *
     * @param v
     * @param w
     *
     * @return
     */
    DistanceType &operator()(const VertexType &v, const VertexType &w) {
        return m_matrix[v][w];
    }

    /**
     * @brief constructor from another metric
     *
     * @tparam OtherMetrics
     * @param other
     * @param xrange
     * @param yrange
     */
    template <typename OtherMetrics, typename XRange, typename YRange>
    rectangle_array_metric(const OtherMetrics &other, XRange && xrange
                           , YRange && yrange)
        : rectangle_array_metric(boost::distance(xrange),
                                 boost::distance(yrange)) {
        int i = 0;
        for (auto && v : xrange) {
            int j = 0;
            for (auto && w : yrange) {
                m_matrix[i][j] = other(v, w);
                ++j;
            }
            ++i;
        }
    }

    /**
     * @brief operator=
     *
     * @param am
     *
     * @return
     */
    rectangle_array_metric &operator=(const rectangle_array_metric &am) {
        auto shape = am.m_matrix.shape();
        std::vector<std::size_t> dim(shape, shape + DIM_NR);
        m_matrix.resize(dim);
        m_matrix = am.m_matrix;
        return *this;
    }

    ///operator==
    bool operator==(const rectangle_array_metric & other) const {
        return m_matrix == other.m_matrix;
    }

  protected:
    /**
     * @brief dimention of multi array
     */
    static const int DIM_NR = 2;
    typedef boost::multi_array<DistanceType, DIM_NR> matrix_type;
    /// matrix with data
    matrix_type m_matrix;
};



/**
 * @brief this metric is rectangle_array_metric with N == M.
 *
 * @tparam DistanceTypeParam
 */
template <typename DistanceTypeParam>
class array_metric : public rectangle_array_metric<DistanceTypeParam> {
    typedef rectangle_array_metric<DistanceTypeParam> base;

  public:
    /**
     * @brief constructor
     *
     * @param N
     */
    array_metric(int N = 0) : base(N, N) {}

    /**
     * @brief returns N
     *
     * @return
     */
    int size() const { return this->m_matrix.size(); }

    /**
     * @brief constructor from another metric
     *
     * @tparam OtherMetrics
     * @tparam Items
     * @param other
     * @param items
     */
    template <typename OtherMetrics, typename Items>
    array_metric(const OtherMetrics &other, Items && items)
        : base(other, items, items) {}
};
}
}
#endif // PAAL_BASIC_METRICS_HPP
