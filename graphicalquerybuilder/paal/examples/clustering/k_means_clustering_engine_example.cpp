/**
 * @file k_means_clustering_engine_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-06-26
 */
//! [K Means Clustering Engine Example]
#include "paal/clustering/k_means_clustering_engine.hpp"
#include "paal/utils/functors.hpp"

#include <vector>
#include <limits>
#include <cmath>

int main() {
    // in this example cetroid is always chosen from the original points
    // in this version of algorithm, user is required to provide functors which are used
    // to run k_medians. That is closest_to and centroid

    using Point = std::vector<double>;

    // sample data
    const int NUMBER_OF_CLUSTER = 2;
    std::vector<Point> points = { { 0, 0 },
                                  { 0, 3 },
                                  { 4, 0 } };
    std::vector<Point> centers(NUMBER_OF_CLUSTER);
    paal::get_random_centers(points, NUMBER_OF_CLUSTER, centers.begin());
    std::vector<std::pair<Point, int>> point_cluster_pairs(points.size());

    //this centroid chooses centers from points
    auto centroid = [&](const std::vector<std::vector<double>> &points) {
        double best_dist = std::numeric_limits<double>::max();
        std::vector<double> result;
        for (auto && center : points) {
            double dist = 1e99;
            for (auto && point : points) {
                dist = std::min(dist, paal::distance_square(point, center));
            }

            if (dist < best_dist) {
                best_dist = dist;
                result = center;
            }
        }
        assert(!result.empty());
        return result;
    };

    //default closest_to
    auto closest_to = [&](const Point & point) {
        return paal::closest_to(point, centers);
    };

    // solution
    paal::k_means(points,
            centers, centroid, closest_to,
            point_cluster_pairs.begin());
    //! [K Means Clustering Engine Example]
}
