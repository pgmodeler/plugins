/**
 * @file k_means_clustering_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-06-25
 */
//! [K Means Clustering Example]

#include <vector>
#include <iostream>

#include "paal/clustering/k_means_clustering.hpp"
#include "paal/utils/functors.hpp"

int main() {
    using Point = std::vector<double>;

    // sample data
    unsigned const int NUMBER_OF_CLUSTER = 2;
    std::vector<Point> points = { { 0, 0 },
                                  { 0, 3 },
                                  { 4, 0 } };
    std::vector<std::pair<Point , int>> point_cluster_pair;
    std::vector<Point> centers(NUMBER_OF_CLUSTER);

    // solution
    paal::get_random_centers(points, NUMBER_OF_CLUSTER, centers.begin());
    paal::k_means(points , centers,
                  back_inserter(point_cluster_pair));

    for (auto i : point_cluster_pair) {
        for(auto && j : i.first) {
            std::cout << j << ",";
        }

        std::cout<< " " << i.second << std::endl;
    }
    //! [K Means Clustering Example]
}
