/**
 * @file k_means_clustering_engine_start_from_clusters_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-06-26
 */
//! [K Means Clustering Engine Example]

#include <vector>
#include <limits>
#include <cmath>

#include "paal/clustering/k_means_clustering.hpp"
#include "paal/utils/functors.hpp"

int main() {

    // sample data
    using Point = std::vector<int>;
    std::vector<Point> points {{ 0, 0 }, { 0, 3 } ,  { 4, 0 }};
    std::vector<std::vector<Point>> clusters = {
        { { 0, 0 }, { 0, 3 } }, { { 4, 0 } }
    };
    std::vector<Point> centers(clusters.size());
    std::vector<std::pair<Point, int>> point_cluster_pair;

    paal::centroids_minimalize_w_c_s_s(clusters, centers.begin());

    // solution
    paal::k_means(points, centers, std::back_inserter(point_cluster_pair));
    //! [K Means Clustering Engine Example]
}
