/**
 * @file k_means_clustering_engine.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-06-26
 */
#ifndef PAAL_K_MEANS_CLUSTERING_ENGINE_HPP
#define PAAL_K_MEANS_CLUSTERING_ENGINE_HPP

#include "paal/utils/type_functions.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/irange.hpp"

#include <boost/range/combine.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include <vector>
#include <random>
#include <iostream>

namespace paal {

/**
 * @param lrange
 * @param rrange
 * @tparam RangeLeft
 * @tparam RangeRight
 */
template <class RangeLeft, class RangeRight>
auto distance_square(RangeLeft && lrange, RangeRight && rrange) {
    assert(!boost::empty(lrange));
    assert(boost::distance(lrange) == boost::distance(rrange));

    //TODO change to sum_functors when generic lambdas appears
    decltype(*std::begin(lrange) * *std::begin(rrange)) dist{};
    for (auto point_pair : boost::combine(lrange, rrange)) {
        auto diff = boost::get<0>(point_pair) - boost::get<1>(point_pair);
        dist += diff * diff;
    }
    return dist;
}

/**
 * @param point
 * @param centers
 * @tparam Point
 * @tparam Centers
 */
template <class Point, class Centers>
auto closest_to(Point && point, Centers && centers){
    using coor_t = range_to_elem_t<Point>;
    auto dist = std::numeric_limits<coor_t>::max();
    int new_center = 0;
    for (auto center : centers | boost::adaptors::indexed()){
        auto new_dist = distance_square(center.value(), point);

        if (new_dist < dist) {
            dist = new_dist;
            new_center = center.index();
        }
    }
    return new_center;
}

///k means visitor
struct k_means_visitor {
    /**
    * @param last_center
    * @param new_center
    * @tparam Center
    * @tparam New_center
    */
    template <class Center, class New_center>
    void move_center(Center &last_center, New_center &new_center) {};
    ///new iteration
    void new_iteration() {};
};

/**
 * @param points
 * @param centers
 * @param centroid functor return centroid of set of samples
 * @param closest_to
 * @param result pairs of point and id of cluster
 * (number from 0,1,2 ...,k-1)
 * @param c_equal
 * @param visitor
 * @tparam Points
 * @tparam Centers
 * @tparam Centroid
 * @tparam ClosestTo
 * @tparam OutputIterator
 * @tparam CentroidEqual
 * @tparam Visitor
 */
template <class Points,
          class Centers,
          class Centroid,
          class ClosestTo, class OutputIterator,
          class CentroidEqual = utils::equal_to,
          class Visitor=k_means_visitor >
auto k_means(Points &&points, Centers & centers,
             Centroid centroid, ClosestTo closest_to,
             OutputIterator result,
             CentroidEqual c_equal = CentroidEqual{},
             Visitor visitor=Visitor{}) {
    using point_t = range_to_elem_t<Points>;
    using points_bag = std::vector<point_t>;

    std::vector<points_bag> cluster_points;
    cluster_points.resize(centers.size());
    bool zm;
    do {
        visitor.new_iteration();
        zm = false;
        boost::for_each(cluster_points, std::mem_fn(&points_bag::clear));

        for (auto && point : points) {
            cluster_points[closest_to(point)].push_back(point);
        }

        for (auto point : cluster_points | boost::adaptors::indexed()) {
            if(point.value().empty()) continue;
            auto && old_center = centers[point.index()];
            auto && new_center = centroid(point.value());
            if (!c_equal(new_center, old_center)) {
                visitor.move_center(old_center, new_center);
                old_center = new_center;
                zm = true;
            }
        }
    } while (zm == true);
    for (int cur_cluster : irange(cluster_points.size())) {
        for (auto const & point : cluster_points[cur_cluster]) {
            *result = std::make_pair(point, cur_cluster);
            ++result;
        }
    }
    return centers;
}

/**
 * @param points
 * @param number_of_centers
 * @tparam Points
 */
template <typename Points, typename OutputIterator, typename RNG = std::default_random_engine>
auto get_random_centers(Points &&points, int number_of_centers, OutputIterator out,
                        RNG && rng = std::default_random_engine{}) {

    std::vector<int> centers(points.size());
    boost::iota(centers, 0);
    std::shuffle(centers.begin(),centers.end(), rng);
    centers.resize(number_of_centers);
    for (auto && center : centers) {
        *out=points[center];
        ++out;
    }
}

/**
 * @param points
 * @param number_of_clusters
 * @tparam Points
 */
template <typename Points, typename RNG = std::default_random_engine>
auto get_random_clusters(Points &&points, int number_of_clusters,
                         RNG && rng = std::default_random_engine{}) {
    std::vector<typename std::decay<Points>::type> clusters(number_of_clusters);
    std::uniform_int_distribution<> dis(0, number_of_clusters - 1);

    for (auto o : points) {
        clusters[distribution(rng)].push_back(o);
    }
    return clusters;
}

} //!paal

#endif /* PAAL_K_MEANS_CLUSTERING_ENGINE_HPP */
