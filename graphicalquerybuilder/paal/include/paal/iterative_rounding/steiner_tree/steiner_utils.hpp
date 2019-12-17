//=======================================================================
// Copyright (c) 2013 Maciej Andrejczuk
//               2014 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_utils.hpp
 * @brief
 * @author Maciej Andrejczuk, Piotr Wygocki
 * @version 1.0
 * @date 2013-08-01
 */
#ifndef PAAL_STEINER_UTILS_HPP
#define PAAL_STEINER_UTILS_HPP

#define BOOST_RESULT_OF_USE_DECLTYPE

#include "paal/data_structures/bimap.hpp"
#include "paal/data_structures/metric/metric_to_bgl.hpp"
#include "paal/utils/irange.hpp"

#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/range/join.hpp>

namespace paal {
namespace ir {

/**
 * Utilities class. Responsible for calculating the cost of given edges.
 */
class steiner_utils {
  public:
    /**
     * Calculates total cost of given edges.
     */
    template <typename Metric, typename Terminals, typename Result>
    static typename data_structures::metric_traits<Metric>::DistanceType
    count_cost(const Result& steiner_vertices, const Terminals& terminals, const Metric& cost_map) {
        using Vertex = typename data_structures::metric_traits<Metric>::VertexType;
        using Dist = typename data_structures::metric_traits<Metric>::DistanceType;

        auto all_elements = boost::range::join(terminals, steiner_vertices);
        paal::data_structures::bimap<Vertex> idx;
        auto g = paal::data_structures::metric_to_bgl_with_index(cost_map,
                all_elements, idx);
        std::vector<std::size_t> pm(all_elements.size());
        boost::prim_minimum_spanning_tree(g, &pm[0]);
        auto idx_m = paal::data_structures::make_metric_on_idx(cost_map, idx);

        Dist cost{};
        for (auto i : irange(pm.size())) {
            cost += idx_m(i, pm[i]);
        }
        return cost;
    }
};

} // ir
} // paal

#endif // PAAL_STEINER_UTILS_HPP
