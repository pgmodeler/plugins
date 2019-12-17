/**
 * @file k_means_clustering.hpp
 * @brief
 * @author Piotr Smulewicz, Piotr Wygocki
 * @version 1.0
 * @date 2014-06-25
 */
#ifndef PAAL_K_MEANS_CLUSTERING_HPP
#define PAAL_K_MEANS_CLUSTERING_HPP

#include "paal/clustering/k_means_clustering_engine.hpp"
#include "paal/utils/irange.hpp"

#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/empty.hpp>

#include <vector>

namespace paal {

/**
 * @brief  return centroid that minimize within-cluster sum of squares
 */
template <typename Cluster, typename OutputIterator>
void centroid_minimalize_w_c_s_s(Cluster && cluster, OutputIterator out) {
    assert(!boost::empty(cluster));
    using point_t = range_to_elem_t<Cluster>;
    using coordinate_t = range_to_elem_t<point_t>;

    auto dim = boost::size(*std::begin(cluster));
    for(auto idx : irange(dim)) {
        coordinate_t res{};
        for (auto && point : cluster) {
            res += std::begin(point)[idx];
        }
        *out = res / boost::size(cluster);
        ++out;
   }
}

/**
 * @brief centroid minimize within cluster sum of squares
 * @param clusters
 * @param out
 * @tparam Clusters
 * @tparam OutputIterator
 */
template <typename Clusters, typename OutputIterator>
void centroids_minimalize_w_c_s_s(Clusters && clusters, OutputIterator out) {
    assert(!boost::empty(clusters));
    assert(!boost::empty(*std::begin(clusters)));

    using cluster_t = range_to_elem_t<Clusters>;
    using point_t = range_to_elem_t<cluster_t>;
    using coordinate_t = range_to_elem_t<point_t>;

    auto size = boost::size(*std::begin(*begin(clusters)));
    for (auto && cluster : clusters) {
        std::vector<coordinate_t> point(size);
        centroid_minimalize_w_c_s_s(cluster, point.begin());
        *out = point;
        ++out;
    }
}

/**
 * @brief this is solve k_means_clustering problem
 * and return vector of cluster
 * example:
 *  \snippet k_means_clustering_example.cpp K Means Clustering Example
 *
 * complete example is k_means_clustering_example.cpp
 * @param points
 * @param centers
 * @param result pairs of point and id of cluster
 * (number form 0,1,2 ...,number_of_cluster-1)
 * @param visitor
 * @tparam Points
 * @tparam OutputIterator
 * @tparam CoordinateType
 * @tparam Visitor
 */
template <typename Points, class Centers, class OutputIterator,
          class Visitor = k_means_visitor>
auto k_means(Points &&points, Centers &&centers, OutputIterator result,
             Visitor visitor = Visitor{}) {
    using point_t = range_to_elem_t<Points>;
    using center_t = range_to_elem_t<Centers>;

    center_t center{ *std::begin(centers) };

    return k_means(
        points, centers,
        [&](std::vector<point_t> const & points)->center_t const & {
            centroid_minimalize_w_c_s_s(points, std::begin(center));
            return center;
        },
        [&](point_t const &point) { return closest_to(point, centers); },
        result, utils::equal_to{}, visitor);
}

} //!paal

#endif /* PAAL_K_MEANS_CLUSTERING_HPP */
